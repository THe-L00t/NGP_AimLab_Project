#include "GameServer.h"
#include <iostream>

namespace {

// 크기가 T와 같으면 T로 본다, 다르면 nullptr
template <typename T>
const T* As(const Packet& packet) {
    if (packet.header.size != sizeof(T)) return nullptr;
    return reinterpret_cast<const T*>(&packet);
}

void LogSizeError(int slot, const Packet& packet) {
    std::cout << "[크기 오류] 자리 " << slot << " 종류 " << static_cast<int>(packet.header.type)
              << " 크기 " << packet.header.size << "\n";
}

} // namespace

// 입력 큐에서 꺼낸 순서대로 불린다
void ProcessPacket(int slot, const Packet& packet) {
    switch (packet.header.type) {
    case PKT_DISCONNECT:
        // RemoveClient (한현우 11/23)
        break;

    case C2S_LOGIN:
        if (const auto* p = As<C2S_LoginPacket>(packet)) {
            // 자리 배정·입장 결과 전송 (한현우 11/11)
            (void)p;
        } else {
            LogSizeError(slot, packet);
        }
        break;

    case C2S_READY:
        if (const auto* p = As<C2S_ReadyPacket>(packet)) {
            // 준비 반영 (안병규 11/12)
            (void)p;
        } else {
            LogSizeError(slot, packet);
        }
        break;

    default:
        std::cout << "[알 수 없는 패킷] 자리 " << slot << " 종류 " << static_cast<int>(packet.header.type) << "\n";
        break;
    }
}
