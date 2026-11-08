#pragma once
// 시작 화면: R 시작, 2·3 조준점 설정
#include "Scene/Scene.h"

class TitleScene : public Scene {
public:
    void onEnter(App& app) override;
    void update(App& app, float dt) override;
    void render(App& app) override;
    const char* name() const override { return "Title"; }
};
