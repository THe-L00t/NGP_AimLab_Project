#pragma once
// 화면이 그릴 게임 상태 (S2C_SNAPSHOT 반영은 이태형 11/16)
#include "Game/GameConfig.h"
#include "Game/GameTypes.h"
#include "Net/Protocol.h"
#include <glm/glm.hpp>
#include <string>
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

// 자리별 플레이어 (네트워크 모드에서 서버 스냅샷으로 채운다)
struct PlayerState {
    bool joined = false;
    bool ready = false;
    std::string nickname;
    float yaw = 0.0f;      // 다른 플레이어 시선 표시용
    float pitch = 0.0f;
    bool trigger = false;
    int fireCount = 0;     // 쏜 횟수 (255 다음 0)
    int hits = 0;
    int roundHits[ROUND_COUNT] = {};
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

    // 네트워크 모드
    PlayerState players[MAX_PLAYERS];
    int requiredPlayers = 0;   // 시작에 필요한 준비 인원

    RoundType round() const { return kRoundOrder[roundIndex]; }
};
