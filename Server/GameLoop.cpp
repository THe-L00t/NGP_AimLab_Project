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
    }
}