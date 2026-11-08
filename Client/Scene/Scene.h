#pragma once
// 씬 기반 클래스
class App;

class Scene {
public:
    virtual ~Scene() = default;
    virtual void onEnter(App&) {}
    virtual void onExit(App&) {}
    virtual void update(App& app, float dt) = 0;
    virtual void render(App& app) = 0;
    virtual const char* name() const = 0;
};
