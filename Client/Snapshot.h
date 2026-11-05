#pragma once
// 화면이 그릴 게임 상태 — 임시.
// 지금 1인 화면이 World에서 읽던 항목만 옮겨 담았다 (3인용·패킷용 항목 없음).
// 11/5 회의에서 Protocol.h의 Snapshot이 확정되면 그것으로 바꾼다.
#include "Game/GameConfig.h"
#include "Game/GameTypes.h"
#include <glm/glm.hpp>
#include <vector>

struct TargetState {
    glm::vec3 pos{ 0.0f };
    float radius = 0.0f;
    float progress = 0.0f;   // Tracking: 연속 조준 시간(초)
};

struct BossState {
    bool alive = false;
    glm::vec3 pos{ 0.0f };
    float radius = 0.0f;
    int hp = 0;
    int maxHp = 0;
    bool shield = false;     // 보호막 중
};

struct Snapshot {
    Phase phase = Phase::Title;
    float phaseTimeLeft = 0.0f;          // 현재 단계의 남은 시간(초)
    int roundIndex = 0;                  // 0~2
    std::vector<TargetState> targets;
    BossState boss;

    int score = 0;                       // 전체 점수
    int roundScore[cfg::kRoundCount] = {};
    int roundGoal = 0;                   // 통과 점수 (Boss는 0)
    bool roundCleared = false;
    int failedRound = -1;                // 통과 못 한 라운드 (-1 = 없음)
    int trackingRemaining = 0;           // Tracking 남은 표적 수
    float shieldProgress = 0.0f;         // 보호막 게이지 0~1
    int shieldsLeft = 0;                 // 남은 보호막 수
    bool shieldTracked = false;          // 보호막 걸린 보스를 따라가고 있나

    RoundType round() const { return kRoundOrder[roundIndex]; }
};