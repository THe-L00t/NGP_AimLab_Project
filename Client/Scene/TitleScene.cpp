#include "Scene/TitleScene.h"
#include "Game/AimMath.h"
#include "Core/App.h"
#include "Core/Palette.h"
#include "Scene/GameScene.h"
#include "Scene/SceneCommon.h"
#include "Snapshot.h"
#include <iostream>

namespace {
// 대기실 (네트워크 모드): 접속 상태, 자리별 닉네임·준비
void drawLobby(App& app) {
    Renderer& r = app.renderer();
    const NetSession& net = *app.net();
    const float cx = r.width() / 2.0f;
    const float cy = r.height() / 2.0f;

    switch (net.state()) {
    case NetSession::State::Connecting:
        r.drawTextCentered(cx, cy + 72.0f, "Connecting to server...", palette::kGray);
        return;
    case NetSession::State::Connected:
        r.drawTextCentered(cx, cy + 72.0f, "Joining...", palette::kGray);
        return;
    case NetSession::State::Failed:
        r.drawTextCentered(cx, cy + 72.0f, "Could not join the server    Press C to retry", palette::kRed);
        return;
    case NetSession::State::Disconnected:
        r.drawTextCentered(cx, cy + 72.0f, "Disconnected from server    Press C to reconnect", palette::kRed);
        return;
    case NetSession::State::LoggedIn:
        break;
    }

    const Snapshot& s = net.snapshot();
    int ready = 0;
    for (const PlayerState& p : s.players) {
        if (p.joined && p.ready) ++ready;
    }
    r.drawTextCentered(cx, cy + 72.0f, scene::format("Ready %d / %d    Press R when ready", ready, s.requiredPlayers),
                       palette::kGreen);

    // 자리별 닉네임·준비
    scene::drawPanel(app, cx - 260.0f, cy - 90.0f, 520.0f, 125.0f, 0.6f);
    float y = cy + 5.0f;
    for (int i = 0; i < MAX_PLAYERS; ++i) {
        const PlayerState& p = s.players[i];
        const bool me = i == net.slot();
        r.drawText(cx - 220.0f, y, scene::format("P%d  %s%s", i + 1, p.joined ? p.nickname.c_str() : "-", me ? "  (YOU)" : ""),
                   me ? palette::kYellow : palette::kWhite);
        if (p.joined) {
            r.drawText(cx + 120.0f, y, p.ready ? "READY" : "WAITING", p.ready ? palette::kGreen : palette::kGray);
        }
        y -= 32.0f;
    }
}
} // namespace

void TitleScene::onEnter(App& app) {
    float yaw = 0.0f, pitch = 0.0f;
    aim::lookAngles(scene::myEye(app), aim::arenaCenter(), yaw, pitch);
    app.camera().setAngles(yaw, pitch);
}

void TitleScene::update(App& app, float) {
    const Snapshot& s = app.session().snapshot();
    if (s.phase != Phase::Title) {
        app.scenes().change(std::make_unique<GameScene>());
        return;
    }

    scene::updateLook(app);
    scene::handleCommonKeys(app, true);

    // 재접속 (이태형 11/25): 끊겼거나 입장에 실패했으면 C로 같은 서버에 다시 접속한다
    if (app.net() && app.input().keyPressed('c')) {
        app.net()->Reconnect();
    }

    if (app.input().keyPressed('r')) {
        if (!app.net()) {   // 1인 모드: 바로 시작
            app.world().start();
            std::cout << "게임 시작\n";
        }
        // 네트워크 모드: 준비 전송 (안병규)
    }
}

void TitleScene::render(App& app) {
    Renderer& r = app.renderer();
    scene::drawWorld(app);

    r.begin2D();
    const float cx = r.width() / 2.0f;
    const float cy = r.height() / 2.0f;
    scene::drawPanel(app, cx - 260.0f, cy + 50.0f, 520.0f, 170.0f, 0.6f);
    r.drawTextCentered(cx, cy + 175.0f, "AIMLAB", palette::kYellow, Font::Large);
    r.drawTextCentered(cx, cy + 148.0f, "Aim trainer", palette::kGray, Font::Small);
    r.drawTextCentered(cx, cy + 110.0f, "1. SHOOTING    2. TRACKING    3. BOSS", palette::kWhite);
    if (app.net()) {
        drawLobby(app);
    } else {
        r.drawTextCentered(cx, cy + 72.0f, "Press R to start", palette::kGreen);
    }

    scene::drawCrosshair(app);
    scene::drawInfoOverlay(app);
    // 네트워크 모드에서 끊겼거나 입장에 실패했으면 R(준비) 대신 C(재접속)를 알려 준다
    const char* key = "R: start";
    if (app.net()) {
        const NetSession::State st = app.net()->state();
        key = st == NetSession::State::Failed || st == NetSession::State::Disconnected ? "C: reconnect" : "R: ready";
    }
    scene::drawHint(app, scene::format("%s    2: crosshair color    3: crosshair shape    M: info    Esc: release mouse    Q: quit", key));
}
