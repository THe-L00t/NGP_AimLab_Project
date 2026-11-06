#include "Server.h"
#include <iostream>

std::array<std::unique_ptr<Client>, MAX_PLAYERS> g_clients;
std::mutex clientsMutex;

namespace {
SOCKET      g_listenSock = INVALID_SOCKET;
std::thread g_gameLoopThread;
}

bool InitServer(uint16_t port) {
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        std::cerr << "WSAStartup 실패\n";
        return false;
    }

    g_listenSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (g_listenSock == INVALID_SOCKET) {
        std::cerr << "socket() 실패: " << WSAGetLastError() << "\n";
        WSACleanup();
        return false;
    }

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(port);
    if (bind(g_listenSock, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) == SOCKET_ERROR) {
        std::cerr << "bind() 실패: " << WSAGetLastError() << "\n";
        closesocket(g_listenSock);
        WSACleanup();
        return false;
    }
    if (listen(g_listenSock, SOMAXCONN) == SOCKET_ERROR) {
        std::cerr << "listen() 실패: " << WSAGetLastError() << "\n";
        closesocket(g_listenSock);
        WSACleanup();
        return false;
    }

    g_gameLoopThread = std::thread(GameLoop);

    std::cout << "포트 " << port << "에서 접속 대기 (최대 " << MAX_PLAYERS << "명)\n";
    return true;
}

void AcceptLoop() {
    while (true) {
        sockaddr_in addr{};
        int addrLen = sizeof(addr);
        SOCKET sock = accept(g_listenSock, reinterpret_cast<sockaddr*>(&addr), &addrLen);
        if (sock == INVALID_SOCKET) {
            std::cerr << "accept() 실패: " << WSAGetLastError() << "\n";
            continue;
        }

        // Nagle 알고리즘을 끈다
        BOOL noDelay = TRUE;
        setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, reinterpret_cast<const char*>(&noDelay), sizeof(noDelay));

        char ip[INET_ADDRSTRLEN] = {};
        inet_ntop(AF_INET, &addr.sin_addr, ip, sizeof(ip));
        const int port = ntohs(addr.sin_port);

        // 빈 자리에 넣는다
        Client* client = nullptr;
        clientsMutex.lock();
        for (int i = 0; i < MAX_PLAYERS; ++i) {
            if (g_clients[i]) continue;
            g_clients[i] = std::make_unique<Client>();
            g_clients[i]->sock = sock;
            g_clients[i]->slot = i;
            client = g_clients[i].get();
            break;
        }
        clientsMutex.unlock();

        if (!client) {
            // 임시: 거부 응답 (이태형 11/12, SendPacket으로 교체)
            S2C_LoginResultPacket result{ MakeHeader<S2C_LoginResultPacket>(S2C_LOGIN_RESULT), -1 };
            send(sock, reinterpret_cast<const char*>(&result), sizeof(result), 0);

            std::cout << "[접속 거부] " << ip << ":" << port << " 자리 없음\n";
            closesocket(sock);
            continue;
        }

        std::cout << "[접속] " << ip << ":" << port << " → 자리 " << client->slot << "\n";
        client->thread = std::thread(ClientThread, client);
    }
}
