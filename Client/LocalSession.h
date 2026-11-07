#pragma once
// 혼자 하는 1인 모드용 Session.
// 같은 프로그램 안의 게임 로직(World)을 진행하고, 화면이 읽을 상태를 Snapshot에 옮겨 둔다.
// 명령(시작·시선·사격·누르기)은 아직 화면이 World에 직접 보낸다.
#include "Session.h"
#include "Snapshot.h"

class World;

class LocalSession : public Session {
public:
    explicit LocalSession(World& world);

    const Snapshot& snapshot() const override { return snapshot_; }
    void update(float dt) override;

private:
    void copyState();   // World 상태 → snapshot_

    World& world_;
    Snapshot snapshot_;
};