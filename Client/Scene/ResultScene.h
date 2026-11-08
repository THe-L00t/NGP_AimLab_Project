#pragma once
// 최종 결과
#include "Scene/Scene.h"

class ResultScene : public Scene {
public:
    void onEnter(App& app) override;
    void update(App& app, float dt) override;
    void render(App& app) override;
    const char* name() const override { return "Result"; }
};
