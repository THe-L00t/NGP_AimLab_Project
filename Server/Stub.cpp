// 임시 코드: 팀원 구현이 합쳐지면 이 파일을 지운다.
#include "Server.h"
#include <chrono>
#include <iostream>

// 임시 (안병규 11/13): 1/30초마다 빈 틱
void GameLoop() {
    while (true) {
        std::this_thread::sleep_for(std::chrono::milliseconds(33));
    }
}
