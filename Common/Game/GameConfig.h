#pragma once
// 게임 규칙 수치

namespace cfg {

// 방
constexpr float kRoomWidth = 40.0f;
constexpr float kRoomHeight = 20.0f;
constexpr float kRoomDepth = 40.0f;
constexpr float kRoomFloorY = -2.0f;
constexpr float kEyeDistance = 14.0f;   // 맵 중앙~눈 수평 거리
constexpr float kEyeHeight = 0.0f;
constexpr float kArenaCenterY = 5.0f;   // 표적 구역 중심 높이

// 카메라
constexpr float kFovYDeg = 55.0f;

// 단계 시간(초)
constexpr float kRoundIntroTime = 3.0f;
constexpr float kRoundSummaryTime = 5.0f;
constexpr float kFinalResultTime = 10.0f;

// 라운드 공통
constexpr int kRoundCount = 3;
constexpr float kTargetRadius = 0.8f;

// 1라운드 Shooting
constexpr float kGridDuration = 30.0f;
constexpr int kGridTargets = 3;
constexpr int kGridGoal = 20;

// 2라운드 Tracking
constexpr float kTrackingDuration = 40.0f;
constexpr int kTrackingTargets = 6;
constexpr int kTrackingGoal = 4;
constexpr float kTrackHoldTime = 2.0f;   // 처치까지 조준 시간
constexpr float kTrackSpeedMin = 2.0f;
constexpr float kTrackSpeedMax = 5.0f;
constexpr float kTrackTurnRate = 0.3f;   // 초당 방향 전환 확률

// 3라운드 Boss
constexpr float kBossDuration = 75.0f;
constexpr int kBossHp = 34;
constexpr float kBossRadius = 1.6f;
constexpr float kBossSpeed = 3.0f;
constexpr float kBossTeleportMinDistance = 6.0f;
constexpr float kBossTeleportInterval[4] = { 2.0f, 1.5f, 1.0f, 0.7f };   // HP 75%·50%·25% 구간별

// 보호막: HP 2/3·1/3에서 켜지고, 누른 채 따라가면 게이지가 찬다
constexpr int kBossShieldCount = 2;
constexpr float kBossShieldHold = 3.0f;    // 깨는 데 필요한 조준 시간
constexpr float kBossShieldDecay = 0.5f;   // 안 볼 때 초당 감소
constexpr float kBossShieldSpeed = 4.5f;

// i번째 보호막이 켜지는 HP
constexpr int bossShieldHp(int maxHp, int i) {
    return maxHp * (kBossShieldCount - i) / (kBossShieldCount + 1);
}

} // namespace cfg
