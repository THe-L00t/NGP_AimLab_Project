#pragma once
// 최종 결과: 전체 점수 + 라운드별 점수. 시간이 지나면 로직이 시작 화면으로 돌려보낸다.
#include "Scene/Scene.h"

class ResultScene : public Scene {
public:
    void onEnter(App& app) override;
    void update(App& app, float dt) override;
    void render(App& app) override;
    const char* name() const override { return "Result"; }
};
