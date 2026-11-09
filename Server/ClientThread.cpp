// 서버: 클라마다 하나씩 도는 수신 스레드 (이태형)
#include "Server.h"
#include "Net/RecvPacket.h"
#include <iostream>

void ClientThread(Client* client) {
    Packet packet;
    while (RecvPacket(client->sock, packet)) {
        // 받은 패킷 → 입력 큐: PushInput 
    }
    std::cout << "[끊김] 자리 " << client->slot << "\n";   // 끊김 알림
}