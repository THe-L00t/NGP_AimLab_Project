#include "Server.h"
#include <windows.h>
#include <cstdlib>
#include <iostream>

// 실행: AimLabServer.exe [포트]
int main(int argc, char** argv) {
    SetConsoleOutputCP(CP_UTF8);

    uint16_t port = SERVER_PORT;
    if (argc > 1) port = static_cast<uint16_t>(std::atoi(argv[1]));

    std::cout << "AimLab Server\n";
    if (!InitServer(port)) return 1;
    AcceptLoop();

    WSACleanup();
    return 0;
}
