// 안병규 담당 파일 (11/13 GameLoop)
#include "GameServer.h"
#include <chrono>
#include <vector>

// 임시: 1/30초마다 빈 틱
void GameLoop() {
    using Clock = std::chrono::steady_clock;
    constexpr float dt = 1.0f / 30.0f;
    const auto interval = std::chrono::duration_cast<Clock::duration>(
        std::chrono::duration<double>(1.0 / 30.0));
    auto nextTick = Clock::now();
    std::vector<Input> inputs;

    while (true) {
        nextTick += interval;
        std::this_thread::sleep_until(nextTick);
        PopInputs(inputs);
        for (const Input& input : inputs) {
            //if (input.disconnected) g_world.leavePlayer(input.slot);
            //else ProcessPacket(input.slot, input.packet);
            ProcessPacket(input.slot, input.packet);
        }
        g_world.tick(dt);
        BroadcastState();
    }
}

void BroadcastState(const std::vector<S2C_EventsPacket>& events) {
    S2C_SnapshotPacket snapshot{};
    BuildSnapshot(snapshot);
    for (int slot = 0; slot < MAX_PLAYERS; ++slot) {
        if (!snapshot.players[slot].joined) continue;
        bool sent = SendTo(slot, reinterpret_cast<const char*>(&snapshot), snapshot.header.size);
        for (const auto& packet : events) {
            if (!sent) break;
            const auto size = sizeof(PacketHeader) + sizeof(packet.count) + packet.count * sizeof(EventData);
            if (packet.header.type != S2C_EVENTS || packet.count == 0 ||
                packet.count > MAX_EVENTS || packet.header.size != size) continue;
            sent = SendTo(slot, reinterpret_cast<const char*>(&packet), packet.header.size);
        }
        if (!sent) {
            Input input{ slot, {} };
            input.packet.header = { static_cast<uint16_t>(sizeof(PacketHeader)), PKT_DISCONNECT };
            PushInput(input);
        }
    }
}
