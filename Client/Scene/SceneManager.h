#pragma once
// 씬 전환은 다음 프레임 시작에 적용한다
#include "Scene/Scene.h"
#include <memory>

class SceneManager {
public:
    void change(std::unique_ptr<Scene> next) { pending_ = std::move(next); }
    void applyPending(App& app);
    Scene* current() const { return current_.get(); }

private:
    std::unique_ptr<Scene> current_;
    std::unique_ptr<Scene> pending_;
};
