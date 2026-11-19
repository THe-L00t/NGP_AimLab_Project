#include <winsock2.h>
#include <ws2tcpip.h>
#include "NetSession.h"
#include "Net/RecvPacket.h"
#include "Net/SendPacket.h"
#include <cmath>
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
    case S2C_SNAPSHOT:       // 받은 스냅샷 반영 (이태형 11/16)
        break;
    case S2C_EVENTS:         // 받은 사건 반영 (이태형 11/18)
        break;
    default:
        std::cout << "[알 수 없는 패킷] 종류 " << int(packet.header.type) << "\n";
        break;
    }
}

bool NetSession::StartNetwork(const char* ip, uint16_t port) {
    CloseNetwork();
    aimSent_ = false;
    nextAimSend_ = {};
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

void NetSession::SendAim(float yaw, float pitch) {
    if (state_ != State::LoggedIn || !std::isfinite(yaw) || !std::isfinite(pitch)) return;
    const auto now = std::chrono::steady_clock::now();
    if (now < nextAimSend_ || (aimSent_ && yaw == lastAimYaw_ && pitch == lastAimPitch_)) return;

    C2S_AimPacket packet{ MakeHeader<C2S_AimPacket>(C2S_AIM), yaw, pitch };
    if (SendPacket(AsSocket(sock_.load()), reinterpret_cast<const char*>(&packet), sizeof(packet))) {
        aimSent_ = true;
        lastAimYaw_ = yaw;
        lastAimPitch_ = pitch;
    }
    nextAimSend_ = std::chrono::steady_clock::now() +
        std::chrono::duration_cast<std::chrono::steady_clock::duration>(std::chrono::duration<double>(1.0 / 30.0));
}

void NetSession::SendFire(float yaw, float pitch) {
    if (state_ != State::LoggedIn || !std::isfinite(yaw) || !std::isfinite(pitch)) return;
    C2S_FirePacket packet{ MakeHeader<C2S_FirePacket>(C2S_FIRE), yaw, pitch };
    SendPacket(AsSocket(sock_.load()), reinterpret_cast<const char*>(&packet), sizeof(packet));
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


