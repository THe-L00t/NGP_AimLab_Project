#include <windows.h>
#include "Core/App.h"
#include <iostream>

// 실행: AimLab.exe [서버 IP] [별명]
int main(int argc, char** argv) {
    SetConsoleOutputCP(CP_UTF8);

    std::cout << "AimLab\n";
    std::cout << "  1라운드 Shooting — 30초 동안 표적 클릭\n";
    std::cout << "  2라운드 Tracking — 움직이는 표적을 좌클릭 누른 채 2초 추적\n";
    std::cout << "  3라운드 Boss     — 순간이동하는 보스의 HP를 0으로\n";
    std::cout << "R 시작, 2 조준점 색, 3 조준점 모양, M 정보, Esc 마우스 놓기, Q 종료\n\n";

    App app;
    return app.run(argc, argv);
}
