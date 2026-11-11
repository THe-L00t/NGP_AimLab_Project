#include <winsock2.h>
#include <ws2tcpip.h>
#include "NetSession.h"
#include "Net/RecvPacket.h"
#include <iostream>

#pragma comment(lib, "ws2_32.lib")

namespace {
SOCKET AsSocket(uintptr_t s) { return static_cast<SOCKET>(s); }
}

NetSession::NetSession(std::string nickname)
    : nickname_(std::move(nickname)) {
}

NetSession::~NetSession() {
    CloseNetwork();
}

void NetSession::update(float) {
    // ProcessServerPacket (이태형 11/11)
}

bool NetSession::StartNetwork(const char* ip, uint16_t port) {
    CloseNetwork();
    ip_ = ip;
    port_ = port;

    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        std::cout << "WSAStartup 실패\n";
        state_ = State::Failed;
        return false;
    }
    wsaReady_ = true;

    state_ = State::Connecting;
    recvThread_ = std::thread(&NetSession::RecvThread, this);
    return true;
}

void NetSession::RecvThread() {
    const SOCKET s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (s == INVALID_SOCKET) {
        std::cout << "socket() 실패: " << WSAGetLastError() << "\n";
        state_ = State::Failed;
        return;
    }
    BOOL noDelay = TRUE;
    setsockopt(s, IPPROTO_TCP, TCP_NODELAY, reinterpret_cast<const char*>(&noDelay), sizeof(noDelay));
    sock_ = static_cast<uintptr_t>(s);
    if (closing_) {   // CloseNetwork가 먼저 불렸으면 직접 닫는다
        const SOCKET mine = AsSocket(sock_.exchange(static_cast<uintptr_t>(INVALID_SOCKET)));
        if (mine != INVALID_SOCKET) closesocket(mine);
        return;
    }

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port_);
    if (inet_pton(AF_INET, ip_.c_str(), &addr.sin_addr) != 1) {
        std::cout << "잘못된 서버 주소: " << ip_ << "\n";
        state_ = State::Failed;
        return;
    }
    if (connect(s, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) == SOCKET_ERROR) {
        std::cout << "서버 접속 실패: " << ip_ << ":" << port_ << " (오류 " << WSAGetLastError() << ")\n";
        state_ = State::Failed;
        return;
    }
    state_ = State::Connected;
    std::cout << "서버 접속: " << ip_ << ":" << port_ << " (" << nickname_ << ")\n";

    // 수신 반복: 패킷 하나씩 받아 패킷 큐에 넣는다 (처리는 Main의 update)
    Packet packet;
    while (RecvPacket(s, packet)) {
        PushPacket(packet);
    }
}

void NetSession::CloseNetwork() {
    closing_ = true;
    const SOCKET s = AsSocket(sock_.exchange(static_cast<uintptr_t>(INVALID_SOCKET)));
    if (s != INVALID_SOCKET) closesocket(s);   // 막힌 connect·recv를 깨운다

    if (recvThread_.joinable()) recvThread_.join();

    if (wsaReady_) {
        WSACleanup();
        wsaReady_ = false;
    }
    closing_ = false;
}
