#pragma once
// 패킷 처리 (GameLoop 스레드 전용, World도 여기서만 만진다)
#include "Server.h"

void ProcessPacket(int slot, const Packet& packet);
