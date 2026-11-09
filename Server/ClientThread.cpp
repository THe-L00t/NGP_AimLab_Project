// 서버: 클라마다 하나씩 도는 수신 스레드 (이태형)
#include "Server.h"
#include <iostream>

void ClientThread(Client* client) {
    char buf[MAX_PACKET_SIZE];
    while (recv(client->sock, buf, sizeof(buf), 0) > 0) {}   // 임시: 끊길 때까지 받아서 버린다
    std::cout << "[끊김] 자리 " << client->slot << "\n";
}