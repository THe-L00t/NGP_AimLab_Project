#include <winsock2.h>
#include <ws2tcpip.h>
#include "NetSession.h"
#include <iostream>

#pragma comment(lib, "ws2_32.lib")

namespace {
    SOCKET sockOf(uintptr_t s) { return static_cast<SOCKET>(s); }
}

NetSession::NetSession(std::string nickname)
    : nickname_(std::move(nickname)) {
}

NetSession::~NetSession() {
    CloseNetwork();
}

void NetSession::update(float) {
    // 받은 패킷 처리는 11/11
}

// 윈속 초기화, 소켓 만들기, 접속
bool NetSession::StartNetwork(const char* ip, uint16_t port) {
    ip_ = ip;
    port_ = port;

    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        std::cout << "WSAStartup 실패\n";
        state_ = State::Failed;
        return false;
    }
    wsaReady_ = true;

    const SOCKET s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (s == INVALID_SOCKET) {
        std::cout << "socket() 실패: " << WSAGetLastError() << "\n";
        state_ = State::Failed;
        return false;
    }
    BOOL noDelay = TRUE;   // 작은 패킷(클릭·시선)을 모아 보내지 않게
    setsockopt(s, IPPROTO_TCP, TCP_NODELAY, reinterpret_cast<const char*>(&noDelay), sizeof(noDelay));
    sock_ = static_cast<uintptr_t>(s);

    return Connect();   // 11/10에 RecvThread 안으로 옮긴다
}

// 서버에 접속 (블로킹: 될 때까지 기다린다)
bool NetSession::Connect() {
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port_);
    if (inet_pton(AF_INET, ip_.c_str(), &addr.sin_addr) != 1) {
        std::cout << "잘못된 서버 주소: " << ip_ << "\n";
        state_ = State::Failed;
        return false;
    }
    if (connect(sockOf(sock_), reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) == SOCKET_ERROR) {
        std::cout << "서버 접속 실패: " << ip_ << ":" << port_ << " (오류 " << WSAGetLastError() << ")\n";
        state_ = State::Failed;
        return false;
    }
    state_ = State::Connected;
    std::cout << "서버 접속: " << ip_ << ":" << port_ << "\n";
    return true;
}

// 소켓 닫기, 윈속 종료
void NetSession::CloseNetwork() {
    if (sockOf(sock_) != INVALID_SOCKET) {
        closesocket(sockOf(sock_));
    }
    sock_ = static_cast<uintptr_t>(INVALID_SOCKET);

    if (wsaReady_) {
        WSACleanup();
        wsaReady_ = false;
    }
}