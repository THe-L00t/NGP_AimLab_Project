#include "Server.h"

namespace {
std::vector<Input> g_inputs;   // ClientThread가 넣고 GameLoop가 꺼낸다
std::mutex inputMutex;
uint32_t g_nextSeq = 0;        // inputMutex로 보호
}

// 잠금 안에서 번호를 매기므로 번호 순서 = 큐에 들어온 순서 = 처리 순서 (먼저 쏜 입력이 먼저 판정)
void PushInput(const Input& input) {
    inputMutex.lock();
    g_inputs.push_back(input);
    g_inputs.back().seq = g_nextSeq++;
    inputMutex.unlock();
}

void PopInputs(std::vector<Input>& out) {
    out.clear();
    inputMutex.lock();
    out.swap(g_inputs);   // 잠금 안에서는 바꿔치기만
    inputMutex.unlock();
}
