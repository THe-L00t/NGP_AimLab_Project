#include <windows.h>  // SetConsoleOutputCP
#include "Core/App.h"
#include <iostream>

int main(int argc, char** argv) {
    // 소스가 UTF-8(/utf-8)이므로 콘솔도 UTF-8로 맞춰야 한글이 깨지지 않는다
    SetConsoleOutputCP(CP_UTF8);

    std::cout << "===========================================\n";
    std::cout << "  AimLab\n";
    std::cout << "===========================================\n";
    std::cout << "시작 화면: R을 누르면 시작\n";
    std::cout << "  1라운드 GridShot  — 30초 동안 표적 클릭\n";
    std::cout << "  2라운드 Tracking  — 움직이는 표적을 좌클릭 누른 채 2초 추적\n";
    std::cout << "  3라운드 Boss      — 순간이동하는 보스의 HP를 0으로\n";
    std::cout << "공통: 2 조준점 색, 3 조준점 모양 (시작·결과 화면), M 정보, Esc 마우스 놓기/잡기, Q 종료\n\n";

    App app;
    return app.run(argc, argv);
}
