#include "LocalSession.h"
#include "Game/World.h"

LocalSession::LocalSession(World& world)
    : world_(world) {
    copyState();
}

void LocalSession::update(float dt) {
    world_.tick(dt);
    copyState();
}

void LocalSession::copyState() {
    Snapshot& s = snapshot_;
    s.phase = world_.phase();
    s.phaseTimeLeft = world_.phaseTimeLeft();
    s.roundIndex = world_.roundIndex();

    s.targets.clear();
    for (const World::Target& t : world_.targets()) {
        s.targets.push_back({ t.pos, t.radius, t.progress });
    }

    const World::Boss& b = world_.boss();
    s.boss = { b.alive, b.pos, b.radius, b.hp, b.maxHp, b.shield };

    s.score = world_.score();
    for (int r = 0; r < cfg::kRoundCount; ++r) s.roundScore[r] = world_.roundScore(r);
    s.roundGoal = world_.roundGoal();
    s.roundCleared = world_.roundCleared();
    s.failedRound = world_.failedRound();
    s.trackingRemaining = world_.trackingRemaining();
    s.shieldProgress = world_.shieldProgress();
    s.shieldsLeft = world_.shieldsLeft();
    s.shieldTracked = world_.shieldTracked();
}