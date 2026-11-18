#pragma once
// 화면이 게임 상태를 받는 창구 (1인: LocalSession, 접속: NetSession)
#include "Game/GameTypes.h"
#include <vector>

struct Snapshot;

class Session {
public:
    virtual ~Session() = default;

    virtual const Snapshot& snapshot() const = 0;
    virtual void update(float dt) = 0;
    virtual void drainEvents(std::vector<GameEvent>& out) = 0;   // 쌓인 사건을 out 뒤에 붙이고 비운다
};
