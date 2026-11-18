#pragma once
// 서버 접속 Session
#include "Session.h"
#include "Snapshot.h"
#include "Net/Protocol.h"
#include <atomic>
#include <cstdint>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

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
    bool SendReady(bool ready);

    State state() const { return state_; }
    int slot() const { return slot_; }   // 내 자리 (0~2), 입장 전 -1

    // ---- 안병규 (NetSend.cpp) ----

private:
    std::string nickname_;
    Snapshot snapshot_;

    // ---- 이태형 ----
    void RecvThread();
    void PushPacket(const Packet& packet);       // 패킷 큐에 넣기 (RecvThread)
    void PopPackets(std::vector<Packet>& out);   // 패킷 큐를 통째로 꺼내기 (Main)
    void ProcessServerPacket(const Packet& packet);   // 받은 패킷 하나 처리 (Main)
    void ApplySnapshot(const S2C_SnapshotPacket& p);   // 받은 스냅샷 → snapshot_ (Main)
    void ApplyEvents(const Packet& packet);            // 받은 사건 → events_ (Main)

    std::string ip_;
    uint16_t port_ = 0;
    bool wsaReady_ = false;
    int slot_ = -1;   // 입장 결과로 받은 내 자리 (Main만 씀)

    std::atomic<uintptr_t> sock_{ ~uintptr_t(0) };  //SOCKET, ~0 = INVALID_SOCKET
    std::atomic<bool> closing_{ false };
    std::atomic<State> state_{ State::Connecting };
    std::thread recvThread_;

    std::mutex packetMutex_;        // packets_ 보호
    std::vector<Packet> packets_;   // 클라 패킷 큐: RecvThread가 넣고 Main이 꺼낸다

    // 받은 사건 큐: Main이 넣고(ApplyEvents) 화면이 꺼낸다. 스레드 사이 잠금은 패킷 큐가 맡는다
    std::vector<GameEvent> events_;

    // ---- 안병규 ----
    bool SendLogin();
};
