#pragma once
// 게임 로직: 표적·보스 이동, 명중 판정, 점수, 라운드 진행
#include "GameConfig.h"
#include "GameTypes.h"
#include <glm/glm.hpp>
#include <random>
#include <vector>

class World {
public:
    struct PlayerInput {
        float yaw = 0.0f;
        float pitch = 0.0f;
        bool triggerHeld = false;
    };

    struct Target {
        glm::vec3 pos{ 0.0f };
        glm::vec3 vel{ 0.0f };
        float radius = cfg::kTargetRadius;
        float progress = 0.0f;    // Tracking 연속 조준 시간(초)
    };

    struct Boss {
        bool alive = false;
        glm::vec3 pos{ 0.0f };
        glm::vec3 vel{ 0.0f };
        float radius = cfg::kBossRadius;
        int hp = 0;
        int maxHp = 0;
        float teleportTimer = 0.0f;
        bool shield = false;
        float shieldHold = 0.0f;  // 보호막 게이지(초)
        int shieldsDone = 0;
    };

    World();

    // 자리
    int joinPlayer();             // 첫 빈 자리, 없으면 -1
    bool joinPlayer(int slot);
    bool leavePlayer(int slot);

    // 입력
    void start();
    void setAim(float yawDeg, float pitchDeg);
    void fire();
    void setTrigger(bool held);
    void setAim(int slot, float yawDeg, float pitchDeg);
    void setTrigger(int slot, bool held);

    // 진행
    void tick(float dt);
    void drainEvents(std::vector<GameEvent>& out);

    // 상태 조회
    Phase phase() const { return phase_; }
    float phaseTimeLeft() const;
    int roundIndex() const { return roundIndex_; }
    RoundType round() const { return kRoundOrder[roundIndex_]; }
    const std::vector<Target>& targets() const { return targets_; }
    const Boss& boss() const { return boss_; }
    const PlayerInput* playerInput(int slot) const;
    int score() const { return score_; }
    int roundScore(int round) const { return roundScore_[round]; }
    int roundGoal() const { return roundGoal_; }   // Boss는 0
    bool roundCleared() const;
    int failedRound() const { return failedRound_; }   // -1 = 없음
    int trackingRemaining() const;
    float shieldProgress() const;   // 0~1
    int shieldsLeft() const;
    bool shieldTracked() const { return shieldTracked_; }

    static glm::vec3 areaCenter(RoundType round);   // 라운드 시작 시점이 보는 곳

private:
    struct Bounds {
        glm::vec3 min;
        glm::vec3 max;
    };

    // 단계 전환
    void enterPhase(Phase phase, float duration);
    void startMatch();
    void startRound(int index);
    void endRound();
    void resetToTitle();
    bool roundFinished() const;
    bool goalReached() const;

    // 라운드별
    void spawnGridShot();
    void spawnTracking();
    void spawnBoss();
    Target makeTrackingTarget();
    void updateTracking(float dt);
    void updateBoss(float dt);
    void hitBoss(const glm::vec3& point);
    void teleportBoss();
    int bossStage(int hp) const;
    void raiseShield();
    void breakShield();

    // 보조
    glm::vec3 aimDirection() const;
    void credit();
    int pickTarget(const glm::vec3& origin, const glm::vec3& dir) const;   // 가장 가까운 표적
    glm::vec3 randomFreePosition(const Bounds& b, float minDistance, int ignoreIndex);
    glm::vec3 randomIn(const Bounds& b);
    float randf(float lo, float hi);
    void clearEntities();

    static Bounds gridBounds();
    static Bounds trackingBounds();
    static Bounds bossBounds();
    static void bounceInside(glm::vec3& pos, glm::vec3& vel, float radius, const Bounds& b);

    Phase phase_ = Phase::Title;
    float phaseTime_ = 0.0f;
    int roundIndex_ = 0;

    static constexpr int kMaxPlayers = 3;
    static constexpr int kLocalSlot = 0;   // 1인 모드 자리
    static bool validSlot(int slot) { return slot >= 0 && slot < kMaxPlayers; }
    bool isPlayerJoined(int slot) const { return validSlot(slot) && playerJoined_[slot]; }
    PlayerInput playerInputs_[kMaxPlayers]{};
    bool playerJoined_[kMaxPlayers]{};

    std::vector<Target> targets_;
    Boss boss_;
    bool shieldTracked_ = false;

    int score_ = 0;
    int roundScore_[cfg::kRoundCount] = {};
    int roundGoal_ = 0;
    bool lastRoundCleared_ = false;
    int failedRound_ = -1;
    int trackKilled_ = 0;

    std::mt19937 rng_;
    std::vector<GameEvent> events_;
};
