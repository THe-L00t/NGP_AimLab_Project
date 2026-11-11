// 안병규 담당 파일 (11/13 GameLoop)
#include "Server.h"
#include <chrono>

// 임시: 1/30초마다 빈 틱
void GameLoop() {
    while (true) {
        std::this_thread::sleep_for(std::chrono::milliseconds(33));
    }
}
