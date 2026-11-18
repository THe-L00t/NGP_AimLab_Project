#include <winsock2.h>
#include <ws2tcpip.h>
#include "NetSession.h"
#include "Net/RecvPacket.h"
#include "Net/SendPacket.h"
#include <cstring>
#include <iostream>
#include <cstring>

#pragma comment(lib, "ws2_32.lib")

namespace {
SOCKET AsSocket(uintptr_t s) { return static_cast<SOCKET>(s); }
}

NetSession::NetSession(std::string nickname)
    : nickname_(std::move(nickname)) {
}

NetSession::~NetSession() {
    CloseNetwork();
}

void NetSession::update(float) {
    std::vector<Packet> packets;
    PopPackets(packets);   // RecvThread가 넣어 둔 패킷을 통째로 꺼낸다
    for (const Packet& packet : packets) ProcessServerPacket(packet);
}

// 받은 패킷 하나를 종류별로 처리한다 (Main 스레드)
void NetSession::ProcessServerPacket(const Packet& packet) {
    switch (packet.header.type) {
    case S2C_LOGIN_RESULT: {   // 입장 결과 반영
        if (packet.header.size != sizeof(S2C_LoginResultPacket)) {
            std::cout << "[크기 오류] 입장 결과 " << packet.header.size << "B\n";
            break;
        }
        S2C_LoginResultPacket result;
        if (state_ != State::Connected) {   // 입장 결과는 한 번만 반영한다
            std::cout << "[무시] 입장 결과 (이미 받음)\n";
            break;
        }
        std::memcpy(&result, &packet, sizeof(result));
        if (result.slot < 0) {   // 입장 거부: 자리 없음
            std::cout << "[입장 거부] 자리 없음\n";
            state_ = State::Failed;
            CloseNetwork();   // 서버가 기다리지 않도록 바로 닫는다
            break;
        }
        slot_ = result.slot;
        state_ = State::LoggedIn;
        std::cout << "[입장] 자리 " << slot_ << "\n";
        break;
    }
    case S2C_SNAPSHOT: {     // 받은 스냅샷 반영 (이태형 11/16)
        if (packet.header.size != sizeof(S2C_SnapshotPacket)) {
            std::cout << "[크기 오류] 스냅샷 " << packet.header.size << "B\n";
            break;
        }
        S2C_SnapshotPacket snap;
        std::memcpy(&snap, &packet, sizeof(snap));
        ApplySnapshot(snap);
        break;
    }
    case S2C_EVENTS:         // 받은 사건 반영 (이태형 11/18)
        ApplyEvents(packet);
        break;
    default:
        std::cout << "[알 수 없는 패킷] 종류 " << int(packet.header.type) << "\n";
        break;
    }
}

static_assert(ROUND_COUNT == cfg::kRoundCount, "패킷과 화면의 라운드 수가 같아야 한다");

// 받은 스냅샷을 화면이 쓰는 Snapshot으로 옮긴다
// Main 스레드(update)에서만 불리므로 snapshot_에는 잠금이 없다 (잠금은 패킷 큐가 맡는다)
void NetSession::ApplySnapshot(const S2C_SnapshotPacket& p) {
    // 화면이 배열 칸 번호로 쓰는 값이 범위 밖이면 통째로 버린다
    if (p.phase > static_cast<uint8_t>(Phase::FinalResult) || p.roundIndex >= ROUND_COUNT ||
        p.failedRound < -1 || p.failedRound >= ROUND_COUNT || p.targetCount > MAX_TARGETS) {
        std::cout << "[잘못된 스냅샷] 단계 " << int(p.phase) << " 라운드 " << int(p.roundIndex)
                  << " 실패 라운드 " << int(p.failedRound) << " 표적 " << int(p.targetCount) << "\n";
        return;
    }

    Snapshot& s = snapshot_;
    s.phase = static_cast<Phase>(p.phase);
    s.phaseTimeLeft = p.phaseTimeLeft;
    s.roundIndex = p.roundIndex;

    s.score = p.teamScore;
    for (int r = 0; r < ROUND_COUNT; ++r) s.roundScore[r] = p.roundTeamScore[r];
    s.roundGoal = p.roundGoal;
    s.roundCleared = p.roundCleared != 0;
    s.failedRound = p.failedRound;
    s.trackingRemaining = p.trackingRemaining;

    // 표적
    s.targets.clear();
    for (int i = 0; i < p.targetCount; ++i) {
        const TargetData& t = p.targets[i];
        s.targets.push_back({ glm::vec3(t.x, t.y, t.z), t.radius, t.progress });
    }

    // 보스 (크기는 늘 같아서 서버가 보내지 않는다)
    const BossData& b = p.boss;
    s.boss.alive = b.alive != 0;
    s.boss.pos = glm::vec3(b.x, b.y, b.z);
    s.boss.radius = cfg::kBossRadius;
    s.boss.hp = b.hp;
    s.boss.maxHp = b.maxHp;
    s.boss.shield = b.shield != 0;
    s.shieldsLeft = b.shieldsLeft;
    s.shieldTracked = b.shieldTracked != 0;
    s.shieldProgress = b.shieldProgress;

    // 자리별 플레이어
    s.requiredPlayers = p.requiredPlayers;
    for (int i = 0; i < MAX_PLAYERS; ++i) {
        const PlayerData& d = p.players[i];
        PlayerState& ps = s.players[i];
        ps.joined = d.joined != 0;
        ps.ready = d.ready != 0;
        char name[NAME_SIZE] = {};
        std::memcpy(name, d.nickname, NAME_SIZE - 1);   // 끝 널 보장
        ps.nickname = name;
        ps.yaw = d.yaw;
        ps.pitch = d.pitch;
        ps.trigger = d.trigger != 0;
        ps.fireCount = d.fireCount;
        ps.hits = d.hits;
        for (int r = 0; r < ROUND_COUNT; ++r) ps.roundHits[r] = d.roundHits[r];
    }
}

// 받은 사건을 사건 큐에 넣는다. 개수만큼만 오므로 크기는 4 + 개수 × 14
void NetSession::ApplyEvents(const Packet& packet) {
    constexpr size_t kHeadSize = sizeof(PacketHeader) + 1;   // 헤더 + 개수
    const size_t size = packet.header.size;
    if (size < kHeadSize || size > sizeof(S2C_EventsPacket)) {
        std::cout << "[크기 오류] 사건 " << size << "B\n";
        return;
    }
    S2C_EventsPacket p{};
    std::memcpy(&p, &packet, size);
    if (p.count > MAX_EVENTS || size != kHeadSize + p.count * sizeof(EventData)) {
        std::cout << "[크기 오류] 사건 " << size << "B " << int(p.count) << "개\n";
        return;
    }

    // 없는 종류가 섞여 있으면 통째로 버린다
    for (int i = 0; i < p.count; ++i) {
        if (p.events[i].kind > static_cast<uint8_t>(EventKind::BossShieldBroken)) {
            std::cout << "[잘못된 사건] 종류 " << int(p.events[i].kind) << "\n";
            return;
        }
    }

    for (int i = 0; i < p.count; ++i) {
        const EventData& e = p.events[i];
        events_.push_back({ static_cast<EventKind>(e.kind), glm::vec3(e.x, e.y, e.z) });
    }
}

bool NetSession::StartNetwork(const char* ip, uint16_t port) {
    CloseNetwork();
    ip_ = ip;
    port_ = port;

    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        std::cout << "WSAStartup 실패\n";
        state_ = State::Failed;
        return false;
    }
    wsaReady_ = true;

    state_ = State::Connecting;
    recvThread_ = std::thread(&NetSession::RecvThread, this);
    return true;
}

void NetSession::RecvThread() {
    const SOCKET s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (s == INVALID_SOCKET) {
        std::cout << "socket() 실패: " << WSAGetLastError() << "\n";
        state_ = State::Failed;
        return;
    }
    BOOL noDelay = TRUE;
    setsockopt(s, IPPROTO_TCP, TCP_NODELAY, reinterpret_cast<const char*>(&noDelay), sizeof(noDelay));
    sock_ = static_cast<uintptr_t>(s);
    if (closing_) {   // CloseNetwork가 먼저 불렸으면 직접 닫는다
        const SOCKET mine = AsSocket(sock_.exchange(static_cast<uintptr_t>(INVALID_SOCKET)));
        if (mine != INVALID_SOCKET) closesocket(mine);
        return;
    }

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port_);
    if (inet_pton(AF_INET, ip_.c_str(), &addr.sin_addr) != 1) {
        std::cout << "잘못된 서버 주소: " << ip_ << "\n";
        state_ = State::Failed;
        return;
    }
    if (connect(s, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) == SOCKET_ERROR) {
        std::cout << "서버 접속 실패: " << ip_ << ":" << port_ << " (오류 " << WSAGetLastError() << ")\n";
        state_ = State::Failed;
        return;
    }
    std::cout << "서버 접속: " << ip_ << ":" << port_ << " (" << nickname_ << ")\n";

    if (!SendLogin()) {
        std::cout << "입장 요청 전송 실패\n";
        state_ = State::Failed;
        return;
    }

    state_ = State::Connected;

    // 수신 반복: RecvPacket → PushPacket (이태형 11/11)
    // 수신 반복: 패킷 하나씩 받아 패킷 큐에 넣는다 (처리는 Main의 update)
    Packet packet;
    while (RecvPacket(s, packet)) {
        PushPacket(packet);
    }
}

bool NetSession::SendLogin() {
    if (nickname_.empty() || nickname_.size() >= NAME_SIZE) return false;

    C2S_LoginPacket packet{};
    packet.header = MakeHeader<C2S_LoginPacket>(C2S_LOGIN);
    std::memcpy(packet.nickname, nickname_.data(), nickname_.size());
    return SendPacket(AsSocket(sock_.load()), reinterpret_cast<const char*>(&packet), sizeof(packet));
}

bool NetSession::SendReady(bool ready) {
    const State current = state_;
    if (current != State::Connected && current != State::LoggedIn) return false;

    C2S_ReadyPacket packet{ MakeHeader<C2S_ReadyPacket>(C2S_READY), static_cast<uint8_t>(ready) };
    return SendPacket(AsSocket(sock_.load()), reinterpret_cast<const char*>(&packet), sizeof(packet));
}

void NetSession::CloseNetwork() {
    closing_ = true;
    const SOCKET s = AsSocket(sock_.exchange(static_cast<uintptr_t>(INVALID_SOCKET)));
    if (s != INVALID_SOCKET) closesocket(s);   // 막힌 connect·recv를 깨운다

    if (recvThread_.joinable()) recvThread_.join();

    if (wsaReady_) {
        WSACleanup();
        wsaReady_ = false;
    }
    closing_ = false;
}


