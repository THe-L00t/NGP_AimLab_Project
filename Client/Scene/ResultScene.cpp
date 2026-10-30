#include "Scene/ResultScene.h"
#include "Core/App.h"
#include "Core/Palette.h"
#include "Scene/SceneCommon.h"
#include "Scene/TitleScene.h"
#include <cmath>
#include <iostream>

void ResultScene::onEnter(App& app) {
    const World& w = app.world();
    std::cout << "===== 최종 결과: "
              << (w.failedRound() < 0 ? std::string("모두 통과") : "라운드 " + std::to_string(w.failedRound() + 1) + " 실패")
              << " · 점수 " << w.score() << " =====\n";
    std::cout << "  라운드별: " << w.roundScore(0) << " / " << w.roundScore(1) << " / " << w.roundScore(2) << "\n";
}

void ResultScene::update(App& app, float) {
    const World& w = app.world();
    if (w.phase() == Phase::Title) {
        app.scenes().change(std::make_unique<TitleScene>());
        return;
    }
    scene::updateLook(app);
    scene::handleCommonKeys(app, true);
}

void ResultScene::render(App& app) {
    Renderer& r = app.renderer();
    const World& w = app.world();
    scene::drawWorld(app);
    r.begin2D();

    const float cx = r.width() / 2.0f, cy = r.height() / 2.0f;
    const float pw = 520.0f, ph = 300.0f;
    const float left = cx - pw / 2.0f;
    scene::drawPanel(app, left, cy - ph / 2.0f, pw, ph, 0.7f);

    float y = cy + ph / 2.0f - 45.0f;
    const bool allClear = w.failedRound() < 0;
    if (allClear) {
        r.drawTextCentered(cx, y, "ALL CLEAR", palette::kGreen, Font::Large);
    } else {
        const int fr = w.failedRound();
        r.drawTextCentered(cx, y, scene::format("FAILED AT ROUND %d - %s", fr + 1, roundName(kRoundOrder[fr])),
                           palette::kRed, Font::Large);
    }
    y -= 40.0f;
    r.drawTextCentered(cx, y, scene::format("SCORE  %d", w.score()), palette::kWhite, Font::Large);
    y -= 45.0f;

    // 라운드별 점수 (실패한 뒤 라운드는 하지 않았다)
    for (int rr = 0; rr < cfg::kRoundCount; ++rr) {
        const bool played = allClear || rr <= w.failedRound();
        r.drawText(left + 130.0f, y, scene::format("R%d  %s", rr + 1, roundName(kRoundOrder[rr])), palette::kGray);
        r.drawText(left + 360.0f, y, played ? scene::format("%d", w.roundScore(rr)) : std::string("-"),
                   played ? palette::kWhite : palette::kGray);
        y -= 30.0f;
    }

    y -= 15.0f;
    const int sec = (int)std::ceil(w.phaseTimeLeft());
    r.drawTextCentered(cx, y, scene::format("Back to title in %d", sec < 1 ? 1 : sec), palette::kGray);

    scene::drawCrosshair(app);
    scene::drawInfoOverlay(app);
    scene::drawHint(app, "2: crosshair color    3: crosshair shape    M: info    Esc: release mouse    Q: quit");
}
