#pragma once

#include "Session.h"
#include "Snapshot.h"
#include "../Common/Protocol.h"
#include <atomic>
#include <cstdint>
#include <string>

class NetSession : public Session {
public:
    enum class State {
        Connecting,   // 접속 중
        Connected,    // 접속됨
        LoggingIn,    // 입장 요청 보냄
        LoggedIn,     // 자리 받음
        Failed        // 접속·입장 실패, 끊김
    };

    explicit NetSession(std::string nickname);
    ~NetSession() override;

    const Snapshot& snapshot() const override { return snapshot_; }
    void update(float dt) override;

    bool StartNetwork(const char* ip, uint16_t port);
    void CloseNetwork();

private:
    bool Connect();

    std::string nickname_;
    std::string ip_;
    uint16_t port_ = 0;
    uintptr_t sock_ = ~uintptr_t(0);   // SOCKET (~0 = INVALID_SOCKET)
    bool wsaReady_ = false;

    std::atomic<State> state_{ State::Connecting };   // RecvThread(11/10)도 바꾸므로 atomic
    Snapshot snapshot_;                // 받은 게임 상태 (해석은 11/16)
};
