#pragma once
#include <winsock2.h>
#include <ws2tcpip.h>
#include "Net/Protocol.h"
#include "Net/SendPacket.h"
#include <array>
#include <cstdint>
#include <memory>
#include <mutex>
#include <thread>
#include <vector>

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

// 서버 내부 끊김 알림 (PacketType에 0은 없다)
constexpr uint8_t PKT_DISCONNECT = 0;

// 입력 큐 항목: 보낸 자리 + 패킷
struct Input {
    int    slot = -1;
    Packet packet{};
};

void PushInput(const Input& input);       // inputMutex
void PopInputs(std::vector<Input>& out);  // inputMutex, 통째로 꺼낸다

void ClientThread(Client* client);   // 이태형
void GameLoop();                     // 안병규
