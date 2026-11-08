#pragma once
// 프로그램 뼈대: GLUT 창·메인 루프, 각 시스템 소유
#include "Core/Settings.h"
#include "Effects/DebrisSystem.h"
#include "Game/GameTypes.h"
#include "Game/World.h"
#include "Input/InputManager.h"
#include "LocalSession.h"
#include "NetSession.h"
#include "Render/Camera.h"
#include "Render/Renderer.h"
#include "Scene/SceneManager.h"
#include <chrono>
#include <memory>
#include <vector>

class App {
public:
    int run(int argc, char** argv);
    void quit();

    InputManager& input() { return input_; }
    Renderer& renderer() { return renderer_; }
    World& world() { return world_; }
    Session& session() { return session_; }
    Camera& camera() { return camera_; }
    DebrisSystem& debris() { return debris_; }
    Settings& settings() { return settings_; }
    SceneManager& scenes() { return scenes_; }
    const std::vector<GameEvent>& frameEvents() const { return frameEvents_; }

private:
    static void displayCallback();
    static void reshapeCallback(int w, int h);
    static void timerCallback(int);
    static void closeCallback();

    void frame();
    void render();
    void handleEvents();
    void releaseGraphics();

    static App* s_app;

    InputManager input_;
    Renderer renderer_;
    World world_;
    LocalSession session_{ world_ };
    std::unique_ptr<NetSession> net_;
    Camera camera_;
    DebrisSystem debris_;
    Settings settings_;
    SceneManager scenes_;
    std::vector<GameEvent> frameEvents_;

    std::chrono::steady_clock::time_point lastTime_;
    bool quitting_ = false;
    bool graphicsReleased_ = false;
};
