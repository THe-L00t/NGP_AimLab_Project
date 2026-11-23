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

// ---- 한현우 ----
bool InitServer(uint16_t port);
void AcceptLoop();

// 서버 내부 끊김 알림 (PacketType에 0은 없다)
constexpr uint8_t PKT_DISCONNECT = 0;

// 입력 큐 항목: 보낸 자리 + 패킷
struct Input {
    int    slot = -1;
    Packet packet{};
    uint32_t seq = 0;   // 도착 순서 (PushInput이 매긴다)
};

void PushInput(const Input& input);       // inputMutex
void PopInputs(std::vector<Input>& out);  // inputMutex, 통째로 꺼낸다
void RemoveClient(int slot);              // GameLoop 스레드에서만

// ---- 이태형 (ClientThread.cpp) ----
void ClientThread(Client* client);

// ---- 안병규 (GameLoop.cpp) ----
void GameLoop();
void BroadcastState(const std::vector<S2C_EventsPacket>& events = {});
