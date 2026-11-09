#include "Server.h"

namespace {
std::vector<Input> g_inputs;   // ClientThread가 넣고 GameLoop가 꺼낸다
std::mutex inputMutex;
}

void PushInput(const Input& input) {
    inputMutex.lock();
    g_inputs.push_back(input);
    inputMutex.unlock();
}

void PopInputs(std::vector<Input>& out) {
    out.clear();
    inputMutex.lock();
    out.swap(g_inputs);   // 잠금 안에서는 바꿔치기만
    inputMutex.unlock();
}
