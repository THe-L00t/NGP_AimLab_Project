#pragma once
#include <winsock2.h>
#include <ws2tcpip.h>
#include "Net/Protocol.h"
#include <array>
#include <cstdint>
#include <memory>
#include <mutex>
#include <thread>

struct Client {
    SOCKET      sock = INVALID_SOCKET;
    int         slot = -1;
    std::thread thread;   // ClientThread
};

// 칸 번호 = 자리 번호, 빈 칸은 nullptr
extern std::array<std::unique_ptr<Client>, MAX_PLAYERS> g_clients;
extern std::mutex clientsMutex;

bool InitServer(uint16_t port);
void AcceptLoop();

void ClientThread(Client* client);   // 이태형
void GameLoop();                     // 안병규
