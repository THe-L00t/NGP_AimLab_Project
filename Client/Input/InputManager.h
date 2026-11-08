#pragma once
// GLUT 입력 콜백을 받아 상태로 모아 둔다. 씬은 매 프레임 조회만 한다.
#include <glm/glm.hpp>

enum class MouseButton { Left = 0, Middle = 1, Right = 2 };

class InputManager {
public:
    void install(int width, int height);
    void beginFrame();
    void endFrame();                       // 이번 프레임 값 비우기
    void onReshape(int width, int height);

    // 키보드 (영문은 소문자로 조회)
    bool keyDown(unsigned char key) const;
    bool keyPressed(unsigned char key) const;    // 이번 프레임에 새로 눌림
    bool keyReleased(unsigned char key) const;

    // 마우스
    bool mouseDown(MouseButton b) const { return mDown_[(int)b]; }
    bool mousePressed(MouseButton b) const { return mPressed_[(int)b]; }
    bool mouseReleased(MouseButton b) const { return mReleased_[(int)b]; }
    glm::vec2 lookDelta() const { return delta_; }   // 픽셀, x 오른쪽 +, y 위쪽 +

    // 마우스 시점 모드 (커서 숨기고 가운데 고정)
    void setMouseLook(bool enabled);
    bool mouseLookEnabled() const { return lookEnabled_; }
    bool mouseCaptured() const { return captured_; }

private:
    static void onKeyDown(unsigned char key, int x, int y);
    static void onKeyUp(unsigned char key, int x, int y);
    static void onMouse(int button, int state, int x, int y);
    static void onMotion(int x, int y);
    static unsigned char normalize(unsigned char key);

    void handleMotion(int x, int y);
    void setCaptured(bool captured);

    static InputManager* s_instance;

    bool down_[256] = {};
    bool pressed_[256] = {};
    bool released_[256] = {};
    bool mDown_[3] = {};
    bool mPressed_[3] = {};
    bool mReleased_[3] = {};
    glm::vec2 delta_{ 0.0f };

    int width_ = 1200;
    int height_ = 800;
    bool lookEnabled_ = true;      // Esc로 전환
    bool focused_ = false;
    bool captured_ = false;
    bool skipNextMotion_ = true;   // 붙잡은 직후 첫 이동은 버린다
    void* hwnd_ = nullptr;
};
