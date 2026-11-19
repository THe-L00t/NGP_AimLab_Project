#pragma once

// 공용 (서버·클라): 패킷 하나 받기
// recv(MSG_WAITALL)은 요청한 만큼 다 올 때까지 기다리므로 잘려 오거나 붙어 와도 하나씩 나뉜다
#include <winsock2.h>
#include <iostream>
#include "Protocol.h"

inline bool RecvPacket(SOCKET sock, Packet& out) {
    const int headerSize = static_cast<int>(sizeof(PacketHeader));
    const int headerGot = recv(sock, reinterpret_cast<char*>(&out.header), headerSize, MSG_WAITALL);
    if (headerGot != headerSize) {
        // 0 = 상대가 닫음, 음수 = 오류·직접 닫음. 헤더 일부만 받고 끊긴 경우만 따로 남긴다
        if (headerGot > 0) std::cout << "[중간 끊김] 헤더 " << headerGot << "/" << headerSize << "B\n";
        return false;
    }

    const int size = out.header.size;
    if (size < headerSize || size > static_cast<int>(sizeof(Packet))) {
        std::cout << "[크기 오류] " << size << "B\n";   // 다음 패킷 경계를 알 수 없어 더 받지 않는다
        return false;
    }

    const int bodySize = size - headerSize;
    if (bodySize == 0) return true;   // 본문 없는 패킷
    const int bodyGot = recv(sock, out.body, bodySize, MSG_WAITALL);
    if (bodyGot != bodySize) {
        std::cout << "[중간 끊김] 종류 " << int(out.header.type) << " 본문 " << (bodyGot > 0 ? bodyGot : 0) << "/"
                  << bodySize << "B\n";
        return false;
    }
    return true;
}