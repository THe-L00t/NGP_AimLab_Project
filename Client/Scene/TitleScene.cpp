#include "Scene/TitleScene.h"
#include "Game/AimMath.h"
#include "Core/App.h"
#include "Core/Palette.h"
#include "Scene/GameScene.h"
#include "Scene/SceneCommon.h"
#include "Snapshot.h"
#include <iostream>

void TitleScene::onEnter(App& app) {
    float yaw = 0.0f, pitch = 0.0f;
    aim::lookAngles(aim::eyePosition(), aim::arenaCenter(), yaw, pitch);
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

    if (app.input().keyPressed('r')) {
        app.world().start();
        std::cout << "게임 시작\n";
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
    r.drawTextCentered(cx, cy + 72.0f, "Press R to start", palette::kGreen);

    scene::drawCrosshair(app);
    scene::drawInfoOverlay(app);
    scene::drawHint(app, "R: start    2: crosshair color    3: crosshair shape    M: info    Esc: release mouse    Q: quit");
}
