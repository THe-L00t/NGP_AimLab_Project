#include "GameServer.h"
#include <iostream>

// 입력 큐에서 꺼낸 순서대로 불린다
void ProcessPacket(int slot, const Packet& packet) {
    switch (packet.header.type) {
    case PKT_DISCONNECT:
        // RemoveClient (한현우 11/23)
        break;

    default:
        std::cout << "[알 수 없는 패킷] 자리 " << slot << " 종류 " << static_cast<int>(packet.header.type) << "\n";
        break;
    }
}
