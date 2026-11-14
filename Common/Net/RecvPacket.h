#pragma once

// 공용 (서버·클라): 패킷 하나 받기
#include <winsock2.h>
#include <iostream>
#include "Protocol.h"

inline bool RecvPacket(SOCKET sock, Packet& out) {
    const int headerSize = static_cast<int>(sizeof(PacketHeader));
    if (recv(sock, reinterpret_cast<char*>(&out.header), headerSize, MSG_WAITALL) != headerSize) return false;

    const int size = out.header.size;
    if (size < headerSize || size > static_cast<int>(sizeof(Packet))) {
        std::cout << "[크기 오류] " << size << "B\n";
        return false;
    }

    const int bodySize = size - headerSize;
    if (bodySize == 0) return true;   // 본문 없는 패킷
    return recv(sock, out.body, bodySize, MSG_WAITALL) == bodySize;
}