// 임시 코드: 팀원 구현이 합쳐지면 이 파일을 지운다.
#include "Server.h"
#include <chrono>
#include <iostream>

// 임시 (이태형 11/9): 끊길 때까지 받아서 버린다
void ClientThread(Client* client) {
    char buf[MAX_PACKET_SIZE];
    while (recv(client->sock, buf, sizeof(buf), 0) > 0) {}
    std::cout << "[끊김] 자리 " << client->slot << "\n";
}

// 임시 (안병규 11/13): 1/30초마다 빈 틱
void GameLoop() {
    while (true) {
        std::this_thread::sleep_for(std::chrono::milliseconds(33));
    }
}
