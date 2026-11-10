// 클라 패킷 큐: RecvThread가 넣고 Main(update)이 꺼낸다 (이태형)
// 잠금 안에서는 넣기·꺼내기만 한다
#include "NetSession.h"

void NetSession::PushPacket(const Packet& packet) {
    std::lock_guard<std::mutex> lock(packetMutex_);
    packets_.push_back(packet);
}