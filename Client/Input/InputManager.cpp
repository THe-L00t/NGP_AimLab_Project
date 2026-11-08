#include <windows.h>
#include "Input/InputManager.h"
#include "Render/GL.h"

InputManager* InputManager::s_instance = nullptr;

void InputManager::install(int width, int height) {
    s_instance = this;
    width_ = width;
    height_ = height;
    // 다른 창이 활성일 때 마우스를 붙잡지 않기 위해
    hwnd_ = WindowFromDC(wglGetCurrentDC());

    glutIgnoreKeyRepeat(0);
    glutKeyboardFunc(onKeyDown);
    glutKeyboardUpFunc(onKeyUp);
    glutMouseFunc(onMouse);
    glutMotionFunc(onMotion);
    glutPassiveMotionFunc(onMotion);
}

void InputManager::onReshape(int width, int height) {
    width_ = width;
    height_ = height;
}

unsigned char InputManager::normalize(unsigned char key) {
    if (key >= 'A' && key <= 'Z') return (unsigned char)(key - 'A' + 'a');
    return key;
}

bool InputManager::keyDown(unsigned char key) const { return down_[normalize(key)]; }
bool InputManager::keyPressed(unsigned char key) const { return pressed_[normalize(key)]; }
bool InputManager::keyReleased(unsigned char key) const { return released_[normalize(key)]; }

// ---- GLUT 콜백 ----

void InputManager::onKeyDown(unsigned char key, int, int) {
    if (!s_instance) return;
    const unsigned char k = normalize(key);
    if (!s_instance->down_[k]) s_instance->pressed_[k] = true;   // 키 반복 제외
    s_instance->down_[k] = true;
}

void InputManager::onKeyUp(unsigned char key, int, int) {
    if (!s_instance) return;
    const unsigned char k = normalize(key);
    s_instance->down_[k] = false;
    s_instance->released_[k] = true;
}

void InputManager::onMouse(int button, int state, int, int) {
    if (!s_instance || button < 0 || button > 2) return;   // 휠 무시
    if (state == GLUT_DOWN) {
        if (!s_instance->mDown_[button]) s_instance->mPressed_[button] = true;
        s_instance->mDown_[button] = true;
    } else {
        s_instance->mDown_[button] = false;
        s_instance->mReleased_[button] = true;
    }
}

void InputManager::onMotion(int x, int y) {
    if (s_instance) s_instance->handleMotion(x, y);
}

void InputManager::handleMotion(int x, int y) {
    if (!captured_) return;
    const int cx = width_ / 2;
    const int cy = height_ / 2;
    if (x == cx && y == cy) return;   // 가운데로 되돌린 이벤트
    if (skipNextMotion_) {
        skipNextMotion_ = false;
    } else {
        delta_.x += (float)(x - cx);
        delta_.y += (float)(cy - y);
    }
    glutWarpPointer(cx, cy);
}

// ---- 마우스 붙잡기 ----

void InputManager::setMouseLook(bool enabled) {
    lookEnabled_ = enabled;
}

void InputManager::setCaptured(bool captured) {
    if (captured == captured_) return;
    captured_ = captured;
    if (captured_) {
        glutSetCursor(GLUT_CURSOR_NONE);
        skipNextMotion_ = true;
        glutWarpPointer(width_ / 2, height_ / 2);
    } else {
        glutSetCursor(GLUT_CURSOR_LEFT_ARROW);
    }
}

void InputManager::beginFrame() {
    const bool focused = hwnd_ != nullptr && GetForegroundWindow() == (HWND)hwnd_;
    if (focused_ && !focused) {
        // 포커스를 잃으면 누름 상태를 푼다
        for (int i = 0; i < 256; ++i) {
            if (down_[i]) released_[i] = true;
            down_[i] = false;
        }
        for (int i = 0; i < 3; ++i) {
            if (mDown_[i]) mReleased_[i] = true;
            mDown_[i] = false;
        }
    }
    focused_ = focused;
    setCaptured(lookEnabled_ && focused);
}

void InputManager::endFrame() {
    for (int i = 0; i < 256; ++i) pressed_[i] = released_[i] = false;
    for (int i = 0; i < 3; ++i) mPressed_[i] = mReleased_[i] = false;
    delta_ = glm::vec2(0.0f);
}
