#pragma once
// 패킷 처리 (GameLoop 스레드 전용, World도 여기서만 만진다)
#include "Server.h"
#include "Game/World.h"

extern World g_world;   // GameLoop 스레드 전용

void ProcessPacket(int slot, const Packet& packet);
bool SendTo(int slot, const char* data, int size);
void BuildSnapshot(S2C_SnapshotPacket& out);   // World 상태 → S2C_SNAPSHOT
EventData ToEventData(const GameEvent& e);
