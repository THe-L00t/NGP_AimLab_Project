#include "World.h"
#include "AimMath.h"
#include <algorithm>
#include <cmath>

namespace {
constexpr float kPi = 3.14159265358979f;
}

World::World()
    : rng_(std::random_device{}()) {
    resetToTitle();
}

int World::joinPlayer() {
    for (int slot = 0; slot < kMaxPlayers; ++slot) {
        if (joinPlayer(slot)) {
            return slot;
        }
    }
    return -1;
}

bool World::joinPlayer(int slot) {
    if (!validSlot(slot) || playerJoined_[slot]) {
        return false;
    }
    playerJoined_[slot] = true;
    return true;
}

bool World::leavePlayer(int slot) {
    if (!isPlayerJoined(slot)) {
        return false;
    }
    setTrigger(slot, false);
    playerInputs_[slot] = {};
    playerJoined_[slot] = false;
    return true;
}

// ---- 표적 구역 (맵 중앙) ----

World::Bounds World::gridBounds() {
    return { { -4.0f, 1.0f, -4.0f }, { 4.0f, 9.0f, 4.0f } };
}

World::Bounds World::trackingBounds() {
    return { { -6.0f, 0.0f, -6.0f }, { 6.0f, 11.0f, 6.0f } };
}

World::Bounds World::bossBounds() {
    return { { -6.0f, 1.0f, -6.0f }, { 6.0f, 11.0f, 6.0f } };
}

glm::vec3 World::areaCenter(RoundType round) {
    const Bounds b = round == RoundType::GridShot ? gridBounds()
                   : round == RoundType::Tracking ? trackingBounds()
                   : bossBounds();
    return (b.min + b.max) * 0.5f;
}

// ---- 입력 ----

void World::start() {
    if (phase_ == Phase::Title) startMatch();
}

void World::setAim(float yawDeg, float pitchDeg) {
    setAim(kLocalSlot, yawDeg, pitchDeg);
}

void World::setAim(int slot, float yawDeg, float pitchDeg) {
    if (!isPlayerJoined(slot)) return;
    PlayerInput& input = playerInputs_[slot];
    input.yaw = yawDeg;
    input.pitch = aim::clampPitch(pitchDeg);
}

void World::fire() {
    if (!isPlayerJoined(kLocalSlot) || phase_ != Phase::RoundPlaying) return;

    const glm::vec3 origin = aim::eyePosition();
    const glm::vec3 dir = aimDirection();

    switch (round()) {
    case RoundType::GridShot: {
        const int idx = pickTarget(origin, dir);
        if (idx < 0) return;
        events_.push_back({ EventKind::TargetHit, targets_[idx].pos });
        credit();
        // 다른 자리로 옮긴다
        Target& t = targets_[idx];
        t.pos = randomFreePosition(gridBounds(), t.radius * 3.0f, idx);
        break;
    }
    case RoundType::Boss: {
        float t = 0.0f;
        if (boss_.alive && aim::raySphere(origin, dir, boss_.pos, boss_.radius, t)) {
            hitBoss(origin + dir * t);
        }
        break;
    }
    case RoundType::Tracking:
        break;  // Tracking은 setTrigger로 판정
    }
}

void World::setTrigger(bool held) {
    setTrigger(kLocalSlot, held);
}

void World::setTrigger(int slot, bool held) {
    if (!isPlayerJoined(slot)) return;
    playerInputs_[slot].triggerHeld = held;
    if (slot == kLocalSlot && !held) {
        // 떼면 진행도 초기화
        for (auto& t : targets_) t.progress = 0.0f;
    }
}

// ---- 진행 ----

void World::enterPhase(Phase phase, float duration) {
    phase_ = phase;
    phaseTime_ = duration;
}

void World::tick(float dt) {
    phaseTime_ -= dt;

    switch (phase_) {
    case Phase::Title:
        break;   // start() 대기

    case Phase::RoundIntro:
        if (phaseTime_ <= 0.0f) {
            const float duration = round() == RoundType::GridShot ? cfg::kGridDuration
                                 : round() == RoundType::Tracking ? cfg::kTrackingDuration
                                 : cfg::kBossDuration;
            enterPhase(Phase::RoundPlaying, duration);
        }
        break;

    case Phase::RoundPlaying:
        if (round() == RoundType::Tracking) updateTracking(dt);
        else if (round() == RoundType::Boss) updateBoss(dt);
        if (roundFinished()) endRound();
        break;

    case Phase::RoundSummary:
        if (phaseTime_ <= 0.0f) {
            // 실패하면 바로 끝낸다
            if (failedRound_ >= 0) enterPhase(Phase::FinalResult, cfg::kFinalResultTime);
            else startRound(roundIndex_ + 1);
        }
        break;

    case Phase::FinalResult:
        if (phaseTime_ <= 0.0f) resetToTitle();
        break;
    }
}

void World::startMatch() {
    failedRound_ = -1;
    score_ = 0;
    for (int& s : roundScore_) s = 0;
    startRound(0);
}

void World::startRound(int index) {
    roundIndex_ = std::clamp(index, 0, cfg::kRoundCount - 1);
    clearEntities();
    playerInputs_[kLocalSlot].triggerHeld = false;

    switch (round()) {
    case RoundType::GridShot: spawnGridShot(); break;
    case RoundType::Tracking: spawnTracking(); break;
    case RoundType::Boss:     spawnBoss(); break;
    }

    roundGoal_ = round() == RoundType::GridShot ? cfg::kGridGoal
               : round() == RoundType::Tracking ? cfg::kTrackingGoal
               : 0;
    lastRoundCleared_ = false;
    enterPhase(Phase::RoundIntro, cfg::kRoundIntroTime);
}

void World::endRound() {
    lastRoundCleared_ = goalReached();   // 엔티티를 지우기 전에 판정
    if (!lastRoundCleared_) failedRound_ = roundIndex_;
    clearEntities();
    if (roundIndex_ < cfg::kRoundCount - 1) enterPhase(Phase::RoundSummary, cfg::kRoundSummaryTime);
    else enterPhase(Phase::FinalResult, cfg::kFinalResultTime);
}

bool World::goalReached() const {
    switch (round()) {
    case RoundType::GridShot: return roundScore_[roundIndex_] >= roundGoal_;
    case RoundType::Tracking: return trackKilled_ >= roundGoal_;
    case RoundType::Boss:     return boss_.maxHp > 0 && !boss_.alive;
    }
    return false;
}

void World::resetToTitle() {
    clearEntities();
    playerInputs_[kLocalSlot].triggerHeld = false;
    roundIndex_ = 0;
    enterPhase(Phase::Title, 0.0f);
}

bool World::roundFinished() const {
    if (phaseTime_ <= 0.0f) return true;
    switch (round()) {
    case RoundType::Tracking: return trackKilled_ >= cfg::kTrackingTargets;
    case RoundType::Boss:     return !boss_.alive;
    default:                  return false;
    }
}

void World::clearEntities() {
    targets_.clear();
    boss_ = Boss{};
    shieldTracked_ = false;
    trackKilled_ = 0;
}

// ---- 1라운드 GridShot ----

void World::spawnGridShot() {
    for (int i = 0; i < cfg::kGridTargets; ++i) {
        Target t;
        t.radius = cfg::kTargetRadius;
        t.pos = randomFreePosition(gridBounds(), t.radius * 3.0f, -1);
        targets_.push_back(t);
    }
}

// ---- 2라운드 Tracking ----

World::Target World::makeTrackingTarget() {
    Bounds b = trackingBounds();
    const float r = cfg::kTargetRadius;
    b.min += glm::vec3(r);
    b.max -= glm::vec3(r);

    Target t;
    t.radius = r;
    t.pos = randomFreePosition(b, r * 3.0f, -1);

    const float angle = randf(0.0f, 2.0f * kPi);
    const float speed = randf(cfg::kTrackSpeedMin, cfg::kTrackSpeedMax);
    t.vel = glm::vec3(std::cos(angle) * speed, randf(-1.0f, 1.0f) * speed * 0.3f, std::sin(angle) * speed);
    return t;
}

void World::spawnTracking() {
    for (int i = 0; i < cfg::kTrackingTargets; ++i) {
        targets_.push_back(makeTrackingTarget());
    }
}

void World::updateTracking(float dt) {
    const Bounds bounds = trackingBounds();

    // 이동·방향 전환·튕기기
    for (auto& t : targets_) {
        t.pos += t.vel * dt;
        if (randf(0.0f, 1.0f) < cfg::kTrackTurnRate * dt) {
            const float angle = randf(0.0f, 2.0f * kPi);
            const float speed = std::sqrt(t.vel.x * t.vel.x + t.vel.z * t.vel.z);
            t.vel.x = std::cos(angle) * speed;
            t.vel.z = std::sin(angle) * speed;
        }
        bounceInside(t.pos, t.vel, t.radius, bounds);
    }

    // 누르고 있을 때 조준 중인 표적
    const int aimed = playerInputs_[kLocalSlot].triggerHeld ? pickTarget(aim::eyePosition(), aimDirection()) : -1;

    for (size_t i = 0; i < targets_.size(); ++i) {
        if (aimed == (int)i) targets_[i].progress += dt;
        else targets_[i].progress = 0.0f;
    }

    for (size_t i = 0; i < targets_.size();) {
        if (targets_[i].progress < cfg::kTrackHoldTime) {
            ++i;
            continue;
        }
        events_.push_back({ EventKind::TargetKilled, targets_[i].pos });
        credit();
        ++trackKilled_;
        targets_.erase(targets_.begin() + i);
    }
}

// ---- 3라운드 Boss ----

void World::spawnBoss() {
    boss_ = Boss{};
    boss_.alive = true;
    boss_.radius = cfg::kBossRadius;
    boss_.maxHp = cfg::kBossHp;
    boss_.hp = boss_.maxHp;

    Bounds b = bossBounds();
    b.min += glm::vec3(boss_.radius);
    b.max -= glm::vec3(boss_.radius);
    boss_.pos = randomIn(b);

    const float angle = randf(0.0f, 2.0f * kPi);
    boss_.vel = glm::vec3(std::cos(angle), randf(-0.3f, 0.3f), std::sin(angle)) * cfg::kBossSpeed;
    boss_.teleportTimer = cfg::kBossTeleportInterval[0];
}

int World::bossStage(int hp) const {
    if (boss_.maxHp <= 0) return 0;
    const float f = (float)hp / (float)boss_.maxHp;
    if (f > 0.75f) return 0;
    if (f > 0.50f) return 1;
    if (f > 0.25f) return 2;
    return 3;
}

void World::updateBoss(float dt) {
    shieldTracked_ = false;
    if (!boss_.alive) return;
    boss_.pos += boss_.vel * dt;
    bounceInside(boss_.pos, boss_.vel, boss_.radius, bossBounds());

    if (boss_.shield) {
        // 누른 채 조준하면 게이지가 찬다
        float t = 0.0f;
        shieldTracked_ = playerInputs_[kLocalSlot].triggerHeld && aim::raySphere(aim::eyePosition(), aimDirection(), boss_.pos, boss_.radius, t);
        if (shieldTracked_) boss_.shieldHold += dt;
        else boss_.shieldHold = std::max(0.0f, boss_.shieldHold - cfg::kBossShieldDecay * dt);
        if (boss_.shieldHold >= cfg::kBossShieldHold) breakShield();
        return;   // 보호막 중에는 순간이동 없음
    }

    boss_.teleportTimer -= dt;
    if (boss_.teleportTimer <= 0.0f) teleportBoss();
}

void World::raiseShield() {
    boss_.shield = true;
    boss_.shieldHold = 0.0f;
    const float angle = randf(0.0f, 2.0f * kPi);
    boss_.vel = glm::vec3(std::cos(angle), randf(-0.3f, 0.3f), std::sin(angle)) * cfg::kBossShieldSpeed;
    events_.push_back({ EventKind::BossShieldUp, boss_.pos });
}

void World::breakShield() {
    boss_.shield = false;
    boss_.shieldHold = 0.0f;
    ++boss_.shieldsDone;
    shieldTracked_ = false;
    events_.push_back({ EventKind::BossShieldBroken, boss_.pos });
    teleportBoss();
}

void World::teleportBoss() {
    const glm::vec3 from = boss_.pos;
    Bounds b = bossBounds();
    b.min += glm::vec3(boss_.radius);
    b.max -= glm::vec3(boss_.radius);

    glm::vec3 to = randomIn(b);
    for (int i = 0; i < 20 && glm::length(to - from) < cfg::kBossTeleportMinDistance; ++i) {
        to = randomIn(b);
    }
    boss_.pos = to;

    const float angle = randf(0.0f, 2.0f * kPi);
    boss_.vel = glm::vec3(std::cos(angle), randf(-0.3f, 0.3f), std::sin(angle)) * cfg::kBossSpeed;
    boss_.teleportTimer = cfg::kBossTeleportInterval[bossStage(boss_.hp)];

    events_.push_back({ EventKind::BossTeleport, from });
}

void World::hitBoss(const glm::vec3& point) {
    if (boss_.shield) return;   // 보호막이 막는다

    const int stageBefore = bossStage(boss_.hp);
    --boss_.hp;
    credit();
    events_.push_back({ EventKind::BossHit, point });

    if (boss_.hp <= 0) {
        boss_.hp = 0;
        boss_.alive = false;
        events_.push_back({ EventKind::BossKilled, boss_.pos });
        return;
    }
    // HP 2/3·1/3에서 보호막
    if (boss_.shieldsDone < cfg::kBossShieldCount && boss_.hp <= cfg::bossShieldHp(boss_.maxHp, boss_.shieldsDone)) {
        raiseShield();
        return;
    }
    // HP 25% 구간마다 즉시 순간이동
    if (bossStage(boss_.hp) != stageBefore) teleportBoss();
}

// ---- 보조 ----

glm::vec3 World::aimDirection() const {
    const PlayerInput& input = playerInputs_[kLocalSlot];
    return aim::direction(input.yaw, input.pitch);
}

void World::credit() {
    ++score_;
    ++roundScore_[roundIndex_];
}

int World::pickTarget(const glm::vec3& origin, const glm::vec3& dir) const {
    int best = -1;
    float bestT = 0.0f;
    for (size_t i = 0; i < targets_.size(); ++i) {
        float t = 0.0f;
        if (!aim::raySphere(origin, dir, targets_[i].pos, targets_[i].radius, t)) continue;
        if (best < 0 || t < bestT) {
            best = (int)i;
            bestT = t;
        }
    }
    return best;
}

float World::randf(float lo, float hi) {
    return std::uniform_real_distribution<float>(lo, hi)(rng_);
}

glm::vec3 World::randomIn(const Bounds& b) {
    return glm::vec3(randf(b.min.x, b.max.x), randf(b.min.y, b.max.y), randf(b.min.z, b.max.z));
}

glm::vec3 World::randomFreePosition(const Bounds& b, float minDistance, int ignoreIndex) {
    for (int attempt = 0; attempt < 100; ++attempt) {
        const glm::vec3 p = randomIn(b);
        bool ok = true;
        for (size_t i = 0; i < targets_.size() && ok; ++i) {
            if ((int)i == ignoreIndex) continue;
            if (glm::length(p - targets_[i].pos) < minDistance) ok = false;
        }
        if (ok) return p;
    }
    return (b.min + b.max) * 0.5f;  // 못 찾으면 가운데
}

void World::bounceInside(glm::vec3& pos, glm::vec3& vel, float radius, const Bounds& b) {
    for (int k = 0; k < 3; ++k) {
        if (pos[k] - radius < b.min[k]) {
            pos[k] = b.min[k] + radius;
            vel[k] = std::fabs(vel[k]);
        } else if (pos[k] + radius > b.max[k]) {
            pos[k] = b.max[k] - radius;
            vel[k] = -std::fabs(vel[k]);
        }
    }
}

// ---- 상태 조회 ----

const World::PlayerInput* World::playerInput(int slot) const {
    return isPlayerJoined(slot) ? &playerInputs_[slot] : nullptr;
}

float World::phaseTimeLeft() const {
    return std::max(0.0f, phaseTime_);
}

bool World::roundCleared() const {
    return phase_ == Phase::RoundPlaying ? goalReached() : lastRoundCleared_;
}

int World::trackingRemaining() const {
    const bool trackingActive = round() == RoundType::Tracking &&
                                (phase_ == Phase::RoundIntro || phase_ == Phase::RoundPlaying);
    return trackingActive ? std::max(0, cfg::kTrackingTargets - trackKilled_) : 0;
}

float World::shieldProgress() const {
    return boss_.shield ? std::min(1.0f, boss_.shieldHold / cfg::kBossShieldHold) : 0.0f;
}

int World::shieldsLeft() const {
    return boss_.alive ? cfg::kBossShieldCount - boss_.shieldsDone : 0;
}

void World::drainEvents(std::vector<GameEvent>& out) {
    out.insert(out.end(), events_.begin(), events_.end());
    events_.clear();
}
