// 서버: 클라마다 하나씩 도는 수신 스레드 (이태형)
#include "Server.h"
#include "Net/RecvPacket.h"
#include <iostream>

namespace {
    // 클라가 보내는 패킷의 정해진 크기 (모르는 종류는 0)
    int ClientPacketSize(uint8_t type) {
        switch (type) {
        case C2S_LOGIN:   return static_cast<int>(sizeof(C2S_LoginPacket));
        case C2S_READY:   return static_cast<int>(sizeof(C2S_ReadyPacket));
        case C2S_AIM:     return static_cast<int>(sizeof(C2S_AimPacket));
        case C2S_FIRE:    return static_cast<int>(sizeof(C2S_FirePacket));
        case C2S_TRIGGER: return static_cast<int>(sizeof(C2S_TriggerPacket));
        default:          return 0;
        }
    }
}

void ClientThread(Client* client) {
    // 소켓·자리는 시작할 때 복사해 두고 client는 다시 읽지 않는다
    // (나중에 RemoveClient가 Client를 지워도 지운 메모리를 읽지 않게)
    const SOCKET sock = client->sock;
    const int slot = client->slot;

    Packet packet;
    while (RecvPacket(sock, packet)) {
        // 종류·크기가 맞지 않는 패킷은 버린다 (꺼내 쓰는 쪽은 크기를 믿고 읽는다)
        if (packet.header.size != ClientPacketSize(packet.header.type)) {
            std::cout << "[잘못된 패킷] 자리 " << slot << " 종류 " << int(packet.header.type)
                << " " << packet.header.size << "B\n";
            continue;
        }
        // 받은 패킷 → 입력 큐(꺼내서 처리하는 건 GameLoop)
        Input input;
        input.slot = slot;
        input.packet = packet;
        PushInput(input);
    }
    std::cout << "[끊김] 자리 " << slot << "\n";   // 끊김 알림
}