#pragma once
// 시작 화면: R을 누르면 게임 시작. 마우스로 시점을 돌려 볼 수 있고, 조준점 설정(2·3)을 바꿀 수 있다.
#include "Scene/Scene.h"

class TitleScene : public Scene {
public:
    void onEnter(App& app) override;   // 맵 중앙을 보게 한다
    void update(App& app, float dt) override;
    void render(App& app) override;
    const char* name() const override { return "Title"; }
};
