#pragma once
// 서버 접속 Session
#include "Session.h"
#include "Snapshot.h"
#include "Net/Protocol.h"
#include <atomic>
#include <cstdint>
#include <string>
#include <thread>

class NetSession : public Session {
public:
    enum class State { Connecting, Connected, LoggedIn, Failed };

    explicit NetSession(std::string nickname);
    ~NetSession() override;

    const Snapshot& snapshot() const override { return snapshot_; }
    void update(float dt) override;

    // ---- 이태형 (NetSession.cpp · PacketQueue.cpp) ----
    bool StartNetwork(const char* ip, uint16_t port);   // 재시도에도 사용
    void CloseNetwork();

    State state() const { return state_; }

    // ---- 안병규 (NetSend.cpp) ----

private:
    std::string nickname_;
    Snapshot snapshot_;

    // ---- 이태형 ----
    void RecvThread();

    std::string ip_;
    uint16_t port_ = 0;
    bool wsaReady_ = false;

    std::atomic<uintptr_t> sock_{ ~uintptr_tALID_SOCKET
    std::atomic<bool> closing_{ false };
    std::atomic<State> state_{ State::Connecting };
    std::thread recvThread_;

    // ---- 안병규 ----
    };
