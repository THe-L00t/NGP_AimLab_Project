#pragma once
#include "GameConfig.h"
#include <glm/glm.hpp>

enum class Phase {
    Title,
    RoundIntro,      // 카운트다운
    RoundPlaying,
    RoundSummary,
    FinalResult
};

enum class RoundType { GridShot, Tracking, Boss };   // GridShot = Shooting

constexpr RoundType kRoundOrder[cfg::kRoundCount] = {
    RoundType::GridShot, RoundType::Tracking, RoundType::Boss
};

inline const char* roundName(RoundType r) {
    switch (r) {
    case RoundType::GridShot: return "SHOOTING";
    case RoundType::Tracking: return "TRACKING";
    case RoundType::Boss:     return "BOSS";
    }
    return "?";
}

// 화면 효과용 사건
enum class EventKind {
    TargetHit,
    TargetKilled,
    BossHit,           // pos = 맞은 지점
    BossTeleport,      // pos = 이동 전 위치
    BossKilled,
    BossShieldUp,
    BossShieldBroken
};

struct GameEvent {
    EventKind kind;
    glm::vec3 pos;
};
