// 임시 코드: 팀원 구현이 합쳐지면 이 파일을 지운다.
#include "Server.h"
#include "Game/World.h"
#include <chrono>
#include <cstring>
#include <iostream>

// 임시 입력 큐와 패킷 분기: 팀원 구현이 합쳐지면 교체한다.
namespace {
std::mutex inputMutex;
std::vector<Input> inputQueue;
}

void PushInput(const Input& input) {
    std::lock_guard<std::mutex> lock(inputMutex);
    inputQueue.push_back(input);
}

void PopInputs(std::vector<Input>& out) {
    out.clear();
    std::lock_guard<std::mutex> lock(inputMutex);
    out.swap(inputQueue);
}

void ProcessPacket(int slot, const Packet& packet) {
    if (packet.header.type == C2S_LOGIN) {
        if (packet.header.size == sizeof(C2S_LoginPacket) &&
            packet.body[0] != '\0' && std::memchr(packet.body, '\0', NAME_SIZE)) {
            g_world.joinPlayer(slot);
        }
    } else if (packet.header.type == C2S_READY) {
        ProcessReadyPacket(g_world, slot, packet);
    }
}

// 임시 (이태형 11/9): 패킷을 입력 큐에 넣고, 끊기면 알린다.
void ClientThread(Client* client) {
    constexpr int headerSize = sizeof(PacketHeader);
    while (true) {
        Packet packet{};
        if (recv(client->sock, reinterpret_cast<char*>(&packet.header),
                 headerSize, MSG_WAITALL) != headerSize) break;
        if (packet.header.size < headerSize || packet.header.size > MAX_PACKET_SIZE) break;

        const int bodySize = packet.header.size - headerSize;
        if (bodySize > 0 && recv(client->sock, packet.body, bodySize, MSG_WAITALL) != bodySize) break;
        PushInput(Input{ client->slot, packet });
    }
    PushInput(Input{ client->slot, {}, true });
    std::cout << "[끊김] 자리 " << client->slot << "\n";
}
