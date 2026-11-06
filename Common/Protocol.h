#pragma once
#include <cstdint>

constexpr uint16_t SERVER_PORT = 9000;
constexpr int MAX_PLAYERS = 3;
constexpr int NAME_SIZE = 16;
constexpr int MAX_PACKET_SIZE = 512;

enum PacketType : uint8_t {
    C2S_LOGIN = 1,
    S2C_LOGIN_RESULT = 101,
};

#pragma pack(push, 1)

struct PacketHeader {
    uint16_t size;   // 헤더 포함 전체 크기
    uint8_t  type;   // PacketType
};

struct S2C_LoginResultPacket {
    PacketHeader header;
    int8_t       slot;   // 0~2, 실패 -1
};

// 받은 패킷 하나 (헤더 + 본문)
struct Packet {
    PacketHeader header;
    char         body[MAX_PACKET_SIZE - sizeof(PacketHeader)];
};

#pragma pack(pop)