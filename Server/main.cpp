#include <windows.h>  // SetConsoleOutputCP
#include "Game/World.h"
#include "Net/Protocol.h"   // 패킷 크기 static_assert 검사
#include <iostream>

// AimLab 서버. 게임 판정·계산은 모두 여기서 Common/Game/World로 한다.
// (접속·스레드는 InitServer / AcceptLoop 등으로 이어서 붙인다)
int main() {
    SetConsoleOutputCP(CP_UTF8);

    std::cout << "===========================================\n";
    std::cout << "  AimLab Server\n";
    std::cout << "===========================================\n";

    World world;   // 공용 게임 로직이 서버에서도 빌드되는지 확인
    std::cout << "World 준비 완료 (phase = " << static_cast<int>(world.phase()) << ")\n";
    return 0;
}
