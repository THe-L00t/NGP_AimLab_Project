#include "Server.h"
#include <cstring>

namespace {
std::vector<Input> g_inputs;   // ClientThread가 넣고 GameLoop가 꺼낸다
std::mutex inputMutex;
uint32_t g_nextSeq = 0;        // inputMutex로 보호
}

// 잠금 안에서 번호를 매기므로 번호 순서 = 큐에 들어온 순서 = 처리 순서 (먼저 쏜 입력이 먼저 판정)
void PushInput(const Input& input) {
    Input in = input;

    // 끊김 알림에는 지금 그 자리 연결 번호를 본문에 담는다 (늦게 온 알림 구분)
    // 두 잠금을 동시에 잡지 않도록 clientsMutex를 먼저 풀고 inputMutex를 잡는다
    if (in.packet.header.type == PKT_DISCONNECT && in.slot >= 0 && in.slot < MAX_PLAYERS) {
        uint32_t id = 0;
        clientsMutex.lock();
        if (g_clients[in.slot]) id = g_clients[in.slot]->id;
        clientsMutex.unlock();
        std::memcpy(in.packet.body, &id, sizeof(id));
    }

    inputMutex.lock();
    g_inputs.push_back(in);
    g_inputs.back().seq = g_nextSeq++;
    inputMutex.unlock();
}

void PopInputs(std::vector<Input>& out) {
    out.clear();
    inputMutex.lock();
    out.swap(g_inputs);   // 잠금 안에서는 바꿔치기만
    inputMutex.unlock();
}
