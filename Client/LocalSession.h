#pragma once
// 1인 모드: World를 직접 진행하고 Snapshot에 옮긴다
#include "Session.h"
#include "Snapshot.h"

class World;

class LocalSession : public Session {
public:
    explicit LocalSession(World& world);

    const Snapshot& snapshot() const override { return snapshot_; }
    void update(float dt) override;

private:
    void copyState();

    World& world_;
    Snapshot snapshot_;
};
