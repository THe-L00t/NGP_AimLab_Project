#pragma once
// 게임 진행 단계 · 라운드 종류 · 화면 효과용 사건.
#include "GameConfig.h"
#include <glm/glm.hpp>

// 게임 진행 단계. World가 정하고, 씬은 이 값을 보고 화면을 바꾼다.
enum class Phase {
    Title,           // 시작 화면: R을 누르면 시작
    RoundIntro,      // 라운드 시작 전 카운트다운
    RoundPlaying,    // 라운드 진행 중
    RoundSummary,    // 라운드 결과 표시
    FinalResult      // 3라운드 종료 후 최종 결과
};

enum class RoundType { GridShot, Tracking, Boss };

constexpr RoundType kRoundOrder[cfg::kRoundCount] = {
    RoundType::GridShot, RoundType::Tracking, RoundType::Boss
};

inline const char* roundName(RoundType r) {
    switch (r) {
    case RoundType::GridShot: return "GRIDSHOT";
    case RoundType::Tracking: return "TRACKING";
    case RoundType::Boss:     return "BOSS";
    }
    return "?";
}

// 화면 효과용 일회성 사건 (파편 튀기기 등)
enum class EventKind {
    TargetHit,     // GridShot 명중
    TargetKilled,  // Tracking 처치
    BossHit,       // pos = 맞은 지점
    BossTeleport,  // pos = 순간이동 직전 위치
    BossKilled,
    BossShieldUp,      // 보호막 켜짐 (트래킹 페이즈 시작)
    BossShieldBroken   // 보호막 깨짐
};

struct GameEvent {
    EventKind kind;
    glm::vec3 pos;
};
