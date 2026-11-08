#pragma once
// 인게임: 카운트다운·라운드 진행·라운드 결과
#include "Game/GameTypes.h"
#include "Scene/Scene.h"

class GameScene : public Scene {
public:
    void onEnter(App& app) override;
    void onExit(App& app) override;
    void update(App& app, float dt) override;
    void render(App& app) override;
    const char* name() const override { return "Game"; }

private:
    void drawHud(App& app);
    void drawIntro(App& app);
    void drawSummary(App& app);

    Phase lastPhase_ = Phase::Title;
    bool triggerHeld_ = false;   // 마지막으로 알린 누름 상태
    float bossFlash_ = 0.0f;
};
