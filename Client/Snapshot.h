#pragma once
// 화면이 그릴 게임 상태 (S2C_SNAPSHOT 반영은 이태형 11/16)
#include "Game/GameConfig.h"
#include "Game/GameTypes.h"
#include <glm/glm.hpp>
#include <vector>

struct TargetState {
    glm::vec3 pos{ 0.0f };
    float radius = 0.0f;
    float progress = 0.0f;   // Tracking 조준 시간(초)
};

struct BossState {
    bool alive = false;
    glm::vec3 pos{ 0.0f };
    float radius = 0.0f;
    int hp = 0;
    int maxHp = 0;
    bool shield = false;
};

struct Snapshot {
    Phase phase = Phase::Title;
    float phaseTimeLeft = 0.0f;
    int roundIndex = 0;
    std::vector<TargetState> targets;
    BossState boss;

    int score = 0;
    int roundScore[cfg::kRoundCount] = {};
    int roundGoal = 0;          // Boss는 0
    bool roundCleared = false;
    int failedRound = -1;       // -1 = 없음
    int trackingRemaining = 0;
    float shieldProgress = 0.0f;
    int shieldsLeft = 0;
    bool shieldTracked = false;

    RoundType round() const { return kRoundOrder[roundIndex]; }
};
