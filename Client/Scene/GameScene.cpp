#include "Scene/GameScene.h"
#include "Game/AimMath.h"
#include "Core/App.h"
#include "Core/Palette.h"
#include "Scene/ResultScene.h"
#include "Scene/SceneCommon.h"
#include "Game/World.h"
#include "Snapshot.h"
#include <algorithm>
#include <cmath>
#include <iostream>

namespace {
const char* roundDescription(RoundType r) {
    switch (r) {
    case RoundType::GridShot: return "Click the targets as fast as you can";
    case RoundType::Tracking: return "Hold left mouse on a moving target for 2 seconds";
    case RoundType::Boss:     return "Shoot the boss. When it shields up, hold left mouse on it";
    }
    return "";
}

// 라운드 통과 조건 한 줄
std::string goalText(const Snapshot& s) {
    switch (s.round()) {
    case RoundType::GridShot: return scene::format("GOAL  %d hits in %.0f s", s.roundGoal, cfg::kGridDuration);
    case RoundType::Tracking: return scene::format("GOAL  %d kills in %.0f s", s.roundGoal, cfg::kTrackingDuration);
    case RoundType::Boss:     return scene::format("GOAL  defeat the boss in %.0f s", cfg::kBossDuration);
    }
    return "";
}

int countdownNumber(float timeLeft) {
    const int n = (int)std::ceil(timeLeft);
    return n < 1 ? 1 : n;
}
}

void GameScene::onEnter(App& app) {
    lastPhase_ = Phase::Title;   // 처음 프레임에 '라운드 시작'으로 인식시켜 시점을 표적 쪽으로 맞춘다
    triggerHeld_ = false;
}

void GameScene::onExit(App& app) {
    if (triggerHeld_) app.world().setTrigger(false);
    triggerHeld_ = false;
}

void GameScene::update(App& app, float dt) {
    const Snapshot& s = app.session().snapshot();
    if (s.phase == Phase::FinalResult) {
        app.scenes().change(std::make_unique<ResultScene>());
        return;
    }

    // 라운드가 새로 시작되면 표적이 나오는 구역 가운데를 보게 한다 (원본: 시작할 때 카메라 초기화)
    if (s.phase == Phase::RoundIntro && lastPhase_ != Phase::RoundIntro) {
        float yaw = 0.0f, pitch = 0.0f;
        aim::lookAngles(aim::eyePosition(), World::areaCenter(s.round()), yaw, pitch);
        app.camera().setAngles(yaw, pitch);
        std::cout << "라운드 " << (s.roundIndex + 1) << " " << roundName(s.round()) << "\n";
    }
    if (s.phase == Phase::RoundSummary && lastPhase_ != Phase::RoundSummary) {
        std::cout << "라운드 " << (s.roundIndex + 1) << (s.roundCleared ? " 통과" : " 실패") << " — 점수 "
                  << s.roundScore[s.roundIndex] << " / 목표 " << s.roundGoal << " (합계 " << s.score << ")\n";
    }
    lastPhase_ = s.phase;

    scene::updateLook(app);
    scene::handleCommonKeys(app, false);

    const bool playing = s.phase == Phase::RoundPlaying;
    const RoundType round = s.round();

    // GridShot·Boss: 클릭 한 번 = 한 발
    if (playing && round != RoundType::Tracking && app.input().mousePressed(MouseButton::Left)) {
        app.world().fire();
    }
    // Tracking과 보스 보호막: 누르고 있는 동안 계속 조준 판정 (보스는 보호막 중일 때만 의미가 있다)
    const bool wantTrigger = playing && (round == RoundType::Tracking || round == RoundType::Boss) &&
                             app.input().mouseDown(MouseButton::Left);
    if (wantTrigger != triggerHeld_) {
        app.world().setTrigger(wantTrigger);
        triggerHeld_ = wantTrigger;
    }

    // 보스가 맞으면 잠깐 하얗게 번쩍
    bossFlash_ = std::max(0.0f, bossFlash_ - dt * 6.0f);
    for (const GameEvent& e : app.frameEvents()) {
        if (e.kind == EventKind::BossHit) bossFlash_ = 1.0f;
    }
    // 보호막을 제대로 따라가는 중이면 보스를 밝게 해서 알려 준다
    if (s.shieldTracked) bossFlash_ = std::max(bossFlash_, 0.45f);
}

void GameScene::render(App& app) {
    scene::drawWorld(app, bossFlash_);
    app.renderer().begin2D();
    drawHud(app);

    const Snapshot& s = app.session().snapshot();
    if (s.phase == Phase::RoundIntro) drawIntro(app);
    if (s.phase == Phase::RoundSummary) drawSummary(app);

    scene::drawCrosshair(app);
    scene::drawInfoOverlay(app);
    const char* how = s.round() == RoundType::Tracking ? "Hold left mouse: track"
                    : (s.round() == RoundType::Boss && s.boss.shield) ? "Shield: hold left mouse on the boss"
                    : "Left click: shoot";
    scene::drawHint(app, scene::format("%s    M: info    Esc: release mouse    Q: quit", how));
}

void GameScene::drawHud(App& app) {
    Renderer& r = app.renderer();
    const Snapshot& s = app.session().snapshot();
    const BossState& boss = s.boss;
    const float sw = (float)r.width();
    const float sh = (float)r.height();
    const int ri = s.roundIndex;
    const bool active = s.phase == Phase::RoundIntro || s.phase == Phase::RoundPlaying;

    // ---- 가운데 위: 라운드 · 남은 시간 ----
    r.drawTextCentered(sw / 2.0f, sh - 35.0f,
                       scene::format("ROUND %d/%d   %s", ri + 1, cfg::kRoundCount, roundName(s.round())),
                       palette::kYellow);
    if (s.phase == Phase::RoundPlaying) {
        r.drawTextCentered(sw / 2.0f, sh - 62.0f, scene::format("Time  %.1fs", s.phaseTimeLeft), palette::kWhite);
    }

    // ---- 보스 HP 바 (+ 보호막 지점 표시, 보호막 게이지) ----
    if (s.round() == RoundType::Boss && active && boss.maxHp > 0) {
        const float bw = 420.0f, bh = 18.0f;
        const float bx = sw / 2.0f - bw / 2.0f, by = sh - 100.0f;
        const float fill = std::clamp((float)boss.hp / (float)boss.maxHp, 0.0f, 1.0f);
        r.drawRect(bx - 2.0f, by - 2.0f, bw + 4.0f, bh + 4.0f, glm::vec4(0.0f, 0.0f, 0.0f, 0.7f));
        r.drawRect(bx, by, bw * fill, bh, glm::vec4(0.9f, 0.15f, 0.25f, 1.0f));
        for (int i = 0; i < cfg::kBossShieldCount; ++i) {
            const float f = (float)cfg::bossShieldHp(boss.maxHp, i) / (float)boss.maxHp;
            r.drawRect(bx + bw * f - 1.5f, by - 4.0f, 3.0f, bh + 8.0f, glm::vec4(palette::kShield, 1.0f));
        }
        r.drawTextCentered(sw / 2.0f, by + 4.0f, scene::format("BOSS  %d / %d", boss.hp, boss.maxHp),
                           palette::kWhite, Font::Small);

        if (boss.shield) {
            const float gh = 12.0f, gy = by - 28.0f;
            r.drawRect(bx - 2.0f, gy - 2.0f, bw + 4.0f, gh + 4.0f, glm::vec4(0.0f, 0.0f, 0.0f, 0.7f));
            r.drawRect(bx, gy, bw * s.shieldProgress, gh, glm::vec4(palette::kShield, 1.0f));
            r.drawTextCentered(sw / 2.0f, gy - 24.0f, "SHIELD UP - hold left mouse on the boss", palette::kShield);
        }
    }

    // ---- 왼쪽 위: 점수 · 목표 ----
    scene::drawPanel(app, 20.0f, sh - 92.0f, 250.0f, 72.0f);
    r.drawText(35.0f, sh - 50.0f, scene::format("SCORE  %d", s.score), palette::kYellow);
    if (s.round() != RoundType::Boss && s.roundGoal > 0) {
        std::string goal = scene::format("Goal  %d / %d", s.roundScore[ri], s.roundGoal);
        if (s.round() == RoundType::Tracking && s.phase != Phase::RoundSummary) {
            goal += scene::format("   left %d", s.trackingRemaining);
        }
        if (s.roundCleared) goal += "  CLEAR";
        r.drawText(35.0f, sh - 78.0f, goal, s.roundCleared ? palette::kGreen : palette::kWhite);
    } else if (s.round() == RoundType::Boss && active) {
        r.drawText(35.0f, sh - 78.0f, scene::format("Shields left  %d", s.shieldsLeft), palette::kShield);
    }
}

void GameScene::drawIntro(App& app) {
    Renderer& r = app.renderer();
    const Snapshot& s = app.session().snapshot();
    const float cx = r.width() / 2.0f, cy = r.height() / 2.0f;
    scene::drawPanel(app, cx - 320.0f, cy - 75.0f, 640.0f, 195.0f, 0.5f);
    r.drawTextCentered(cx, cy + 88.0f, scene::format("ROUND %d - %s", s.roundIndex + 1, roundName(s.round())),
                       palette::kYellow, Font::Large);
    r.drawTextCentered(cx, cy + 55.0f, roundDescription(s.round()), palette::kWhite);
    r.drawTextCentered(cx, cy + 26.0f, goalText(s), palette::kGreen);
    // 숫자는 화면 가운데 조준점과 겹치지 않게 아래에
    r.drawTextCentered(cx, cy - 55.0f, scene::format("%d", countdownNumber(s.phaseTimeLeft)), palette::kWhite, Font::Large);
}

void GameScene::drawSummary(App& app) {
    Renderer& r = app.renderer();
    const Snapshot& s = app.session().snapshot();
    const float cx = r.width() / 2.0f, cy = r.height() / 2.0f;
    const int ri = s.roundIndex;
    const bool cleared = s.roundCleared;
    const float ph = 170.0f;
    scene::drawPanel(app, cx - 270.0f, cy - ph / 2.0f, 540.0f, ph, 0.65f);

    float y = cy + ph / 2.0f - 40.0f;
    r.drawTextCentered(cx, y, scene::format("ROUND %d %s", ri + 1, cleared ? "CLEAR" : "FAILED"),
                       cleared ? palette::kGreen : palette::kRed, Font::Large);
    y -= 36.0f;
    r.drawTextCentered(cx, y, scene::format("Score %d / goal %d   (total %d)", s.roundScore[ri], s.roundGoal, s.score),
                       palette::kWhite);
    y -= 36.0f;
    if (!cleared) {
        r.drawTextCentered(cx, y - 8.0f, scene::format("Goal not reached - game over in %d", countdownNumber(s.phaseTimeLeft)),
                           palette::kRed);
    } else if (ri + 1 < cfg::kRoundCount) {
        r.drawTextCentered(cx, y - 8.0f,
                           scene::format("Next: %s in %d", roundName(kRoundOrder[ri + 1]), countdownNumber(s.phaseTimeLeft)),
                           palette::kGray);
    }
}
