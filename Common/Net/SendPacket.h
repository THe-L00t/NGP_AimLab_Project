#pragma once
#include <winsock2.h>

// 패킷 하나를 끝까지 보낸다 (블로킹)
inline bool SendPacket(SOCKET sock, const char* data, int size) {
    if (sock == INVALID_SOCKET || data == nullptr || size <= 0) return false;

    int sent = 0;
    while (sent < size) {
        const int bytes = ::send(sock, data + sent, size - sent, 0);
        if (bytes <= 0) return false;
        sent += bytes;
    }
    return true;
}
