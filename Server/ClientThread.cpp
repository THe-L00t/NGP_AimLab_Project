// 서버: 클라마다 하나씩 도는 수신 스레드 (이태형)
#include "Server.h"
#include "Net/RecvPacket.h"
#include <iostream>

void ClientThread(Client* client) {
    // 소켓·자리는 시작할 때 복사해 두고 client는 다시 읽지 않는다
    // (나중에 RemoveClient가 Client를 지워도 지운 메모리를 읽지 않게)
    const SOCKET sock = client->sock;
    const int slot = client->slot;

    Packet packet;
    while (RecvPacket(sock, packet)) {
        // 받은 패킷 → 입력 큐: PushInput
    }
    std::cout << "[끊김] 자리 " << slot << "\n";   // 끊김 알림
}