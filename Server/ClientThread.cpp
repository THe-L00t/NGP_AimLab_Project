// 이태형 담당 파일 (11/9 ClientThread·RecvPacket)
#include "Server.h"
#include <iostream>

// 임시: 끊길 때까지 받아서 버린다
void ClientThread(Client* client) {
    char buf[MAX_PACKET_SIZE];
    while (recv(client->sock, buf, sizeof(buf), 0) > 0) {}
    std::cout << "[끊김] 자리 " << client->slot << "\n";
}
