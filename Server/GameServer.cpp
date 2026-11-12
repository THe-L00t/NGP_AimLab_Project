#include "GameServer.h"
#include <cstring>
#include <iostream>

World g_world;

namespace {

char g_nicknames[MAX_PLAYERS][NAME_SIZE] = {};   // 자리별 닉네임 (로그인 때 정함)

// 크기가 T와 같으면 T로 본다, 다르면 nullptr
template <typename T>
const T* As(const Packet& packet) {
    if (packet.header.size != sizeof(T)) return nullptr;
    return reinterpret_cast<const T*>(&packet);
}

void LogSizeError(int slot, const Packet& packet) {
    std::cout << "[크기 오류] 자리 " << slot << " 종류 " << static_cast<int>(packet.header.type)
              << " 크기 " << packet.header.size << "\n";
}

// 로그인: 닉네임 정리 후 자리 배정
void HandleLogin(int slot, const C2S_LoginPacket& p) {
    char name[NAME_SIZE] = {};
    std::memcpy(name, p.nickname, NAME_SIZE - 1);   // 끝 널 보장
    if (name[0] == '\0') {   // 비면 PlayerN
        std::memcpy(name, "Player", 6);
        name[6] = static_cast<char>('1' + slot);
    }

    if (!g_world.joinPlayer(slot)) {
        std::cout << "[로그인 실패] 자리 " << slot << "\n";
        // 실패 응답 slot -1 (이태형 입장 거부)
        return;
    }
    std::memcpy(g_nicknames[slot], name, NAME_SIZE);
    std::cout << "[로그인] 자리 " << slot << " " << name << "\n";

    // 입장 결과: 배정한 자리 번호
    S2C_LoginResultPacket result{ MakeHeader<S2C_LoginResultPacket>(S2C_LOGIN_RESULT), static_cast<int8_t>(slot) };
    SendTo(slot, reinterpret_cast<const char*>(&result), sizeof(result));
}

} // namespace

// 한 자리에 보낸다. 잠금 안에서는 소켓만 꺼내고 보내기는 밖에서 한다
bool SendTo(int slot, const char* data, int size) {
    if (slot < 0 || slot >= MAX_PLAYERS) return false;

    SOCKET sock = INVALID_SOCKET;
    clientsMutex.lock();
    if (g_clients[slot]) sock = g_clients[slot]->sock;
    clientsMutex.unlock();

    return SendPacket(sock, data, size);
}

// 입력 큐에서 꺼낸 순서대로 불린다
void ProcessPacket(int slot, const Packet& packet) {
    switch (packet.header.type) {
    case PKT_DISCONNECT:
        // RemoveClient (한현우 11/23)
        break;

    case C2S_LOGIN:
        if (const auto* p = As<C2S_LoginPacket>(packet)) {
            HandleLogin(slot, *p);
        } else {
            LogSizeError(slot, packet);
        }
        break;

    case C2S_READY:
        if (const auto* p = As<C2S_ReadyPacket>(packet)) {
            // 준비 반영 (안병규 11/12)
            (void)p;
        } else {
            LogSizeError(slot, packet);
        }
        break;

    case C2S_AIM:
        if (const auto* p = As<C2S_AimPacket>(packet)) {
            // 시선 반영 (안병규 11/17)
            (void)p;
        } else {
            LogSizeError(slot, packet);
        }
        break;

    case C2S_FIRE:
        if (const auto* p = As<C2S_FirePacket>(packet)) {
            // 발사 판정·먼저 맞힌 점수 (안병규 11/19)
            (void)p;
        } else {
            LogSizeError(slot, packet);
        }
        break;

    case C2S_TRIGGER:
        if (const auto* p = As<C2S_TriggerPacket>(packet)) {
            // 누르기 반영: Tracking·보호막 (안병규 11/23)
            (void)p;
        } else {
            LogSizeError(slot, packet);
        }
        break;

    default:
        std::cout << "[알 수 없는 패킷] 자리 " << slot << " 종류 " << static_cast<int>(packet.header.type) << "\n";
        break;
    }
}

// 스냅샷: 진행 단계·시간·라운드·점수
void BuildSnapshot(S2C_SnapshotPacket& out) {
    out = {};
    out.header = MakeHeader<S2C_SnapshotPacket>(S2C_SNAPSHOT);

    out.phase = static_cast<uint8_t>(g_world.phase());
    out.roundIndex = static_cast<uint8_t>(g_world.roundIndex());
    out.phaseTimeLeft = g_world.phaseTimeLeft();

    out.teamScore = static_cast<uint16_t>(g_world.score());
    for (int r = 0; r < ROUND_COUNT; ++r) {
        out.roundTeamScore[r] = static_cast<uint16_t>(g_world.roundScore(r));
    }
    out.trackingRemaining = static_cast<uint16_t>(g_world.trackingRemaining());
    out.roundGoal = static_cast<uint16_t>(g_world.roundGoal());
    out.roundCleared = g_world.roundCleared() ? 1 : 0;
    out.failedRound = static_cast<int8_t>(g_world.failedRound());

    // 플레이어: 들어온 자리만 채운다
    uint8_t joinedCount = 0;
    for (int slot = 0; slot < MAX_PLAYERS; ++slot) {
        const World::PlayerInput* in = g_world.playerInput(slot);
        if (!in) continue;

        PlayerData& pd = out.players[slot];
        pd.joined = 1;
        std::memcpy(pd.nickname, g_nicknames[slot], NAME_SIZE);
        pd.yaw = in->yaw;
        pd.pitch = in->pitch;
        pd.trigger = in->triggerHeld ? 1 : 0;
        // ready·fireCount·hits·roundHits: 안병규 플레이어별 데이터 추가 후 채운다
        ++joinedCount;
    }
    out.requiredPlayers = joinedCount;   // 들어온 모두가 준비해야 시작
}
