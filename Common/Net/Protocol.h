#pragma once
// 서버·클라 공용 패킷 정의. 헤더 3B(크기 2 + 종류 1) + 종류별 본문.
#include <cstdint>

constexpr uint16_t SERVER_PORT = 9000;

constexpr int MAX_PLAYERS = 3;
constexpr int NAME_SIZE   = 16;
constexpr int ROUND_COUNT = 3;
constexpr int MAX_TARGETS = 8;
constexpr int MAX_EVENTS  = 16;
constexpr int MAX_PACKET_SIZE = 512;

enum PacketType : uint8_t {
    C2S_LOGIN = 1,
    C2S_READY,
    C2S_AIM,
    C2S_FIRE,
    C2S_TRIGGER,

    S2C_LOGIN_RESULT = 101,
    S2C_SNAPSHOT,
    S2C_EVENTS,
};

#pragma pack(push, 1)

struct PacketHeader {
    uint16_t size;   // 헤더 포함 전체 크기
    uint8_t  type;   // PacketType
};

// ---- 클라 → 서버 ----

struct C2S_LoginPacket {
    PacketHeader header;
    char         nickname[NAME_SIZE];
};

struct C2S_ReadyPacket {
    PacketHeader header;
    uint8_t      ready;
};

struct C2S_AimPacket {
    PacketHeader header;
    float        yaw;
    float        pitch;
};

struct C2S_FirePacket {
    PacketHeader header;
    float        yaw;    // 쏜 순간의 시선
    float        pitch;
};

struct C2S_TriggerPacket {
    PacketHeader header;
    uint8_t      held;
    float        yaw;
    float        pitch;
};

// ---- 서버 → 클라 ----

struct S2C_LoginResultPacket {
    PacketHeader header;
    int8_t       slot;   // 0~2, 실패 -1
};

struct PlayerData {
    uint8_t  joined;
    uint8_t  ready;
    char     nickname[NAME_SIZE];
    float    yaw;
    float    pitch;
    uint8_t  trigger;
    uint8_t  fireCount;   // 쏜 횟수 (255 다음 0)
    uint16_t hits;
    uint16_t roundHits[ROUND_COUNT];
};

struct BossData {
    uint8_t  alive;
    float    x, y, z;
    uint16_t hp;
    uint16_t maxHp;
    uint8_t  shield;
    uint8_t  shieldsLeft;
    uint8_t  shieldTracked;
    float    shieldProgress;   // 0~1
};

struct TargetData {
    float   x, y, z;
    float   vx, vy, vz;
    float   radius;
    float   progress;    // Tracking 진행도(초)
    uint8_t aimedMask;   // 조준 중인 자리 비트
};

struct S2C_SnapshotPacket {
    PacketHeader header;
    uint8_t      phase;
    uint8_t      roundIndex;
    uint8_t      requiredPlayers;
    float        phaseTimeLeft;
    uint16_t     teamScore;
    uint16_t     roundTeamScore[ROUND_COUNT];
    uint16_t     trackingRemaining;
    uint16_t     roundGoal;
    uint8_t      roundCleared;
    int8_t       failedRound;   // -1 = 없음
    PlayerData   players[MAX_PLAYERS];
    BossData     boss;
    uint8_t      targetCount;
    TargetData   targets[MAX_TARGETS];
};

struct EventData {
    uint8_t kind;   // EventKind
    int8_t  slot;   // 일으킨 자리, -1 = 없음
    float   x, y, z;
};

// count개만 보낸다 (size = 4 + count × 14)
struct S2C_EventsPacket {
    PacketHeader header;
    uint8_t      count;
    EventData    events[MAX_EVENTS];
};

// 받은 패킷 하나 (헤더 + 본문)
struct Packet {
    PacketHeader header;
    char         body[MAX_PACKET_SIZE - sizeof(PacketHeader)];
};

#pragma pack(pop)

static_assert(sizeof(PacketHeader) == 3, "PacketHeader 3B");
static_assert(sizeof(C2S_LoginPacket) == 19, "C2S_LoginPacket 19B");
static_assert(sizeof(C2S_ReadyPacket) == 4, "C2S_ReadyPacket 4B");
static_assert(sizeof(C2S_AimPacket) == 11, "C2S_AimPacket 11B");
static_assert(sizeof(C2S_FirePacket) == 11, "C2S_FirePacket 11B");
static_assert(sizeof(C2S_TriggerPacket) == 12, "C2S_TriggerPacket 12B");
static_assert(sizeof(S2C_LoginResultPacket) == 4, "S2C_LoginResultPacket 4B");
static_assert(sizeof(PlayerData) == 36, "PlayerData 36B");
static_assert(sizeof(BossData) == 24, "BossData 24B");
static_assert(sizeof(TargetData) == 33, "TargetData 33B");
static_assert(sizeof(EventData) == 14, "EventData 14B");
static_assert(sizeof(S2C_SnapshotPacket) == 421, "S2C_SnapshotPacket 421B");
static_assert(sizeof(S2C_EventsPacket) <= MAX_PACKET_SIZE, "S2C_EventsPacket 크기 초과");
static_assert(sizeof(Packet) == MAX_PACKET_SIZE, "Packet 512B");

// 예: MakeHeader<C2S_AimPacket>(C2S_AIM)
template <typename T>
inline PacketHeader MakeHeader(PacketType type) {
    return PacketHeader{ static_cast<uint16_t>(sizeof(T)), static_cast<uint8_t>(type) };
}
