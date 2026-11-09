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
