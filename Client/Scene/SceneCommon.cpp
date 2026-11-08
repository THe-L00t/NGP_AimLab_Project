#include "Scene/SceneCommon.h"
#include "Game/AimMath.h"
#include "Core/App.h"
#include "Core/Palette.h"
#include "Snapshot.h"
#include <glm/gtc/matrix_transform.hpp>
#include <algorithm>
#include <cstdarg>
#include <cstdio>
#include <iostream>

namespace scene {

std::string format(const char* fmt, ...) {
    char buf[256];
    va_list args;
    va_start(args, fmt);
    std::vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);
    return buf;
}

void updateLook(App& app) {
    const glm::vec2 d = app.input().lookDelta();
    Camera& cam = app.camera();
    if (d.x != 0.0f || d.y != 0.0f) cam.addLook(d.x, d.y);
    cam.setPosition(aim::eyePosition());
    app.world().setAim(cam.yaw(), cam.pitch());
}

void handleCommonKeys(App& app, bool allowSettings) {
    InputManager& in = app.input();
    Settings& st = app.settings();
    if (in.keyPressed('q')) {
        app.quit();
        return;
    }
    if (in.keyPressed('m')) {
        st.infoTimer = 3.0f;
        std::cout << "조준점: " << st.colorName() << " / " << st.shapeName() << "\n";
    }
    if (!allowSettings) return;   // 게임 중에는 변경 불가
    if (in.keyPressed('2')) {
        st.cycleColor();
        st.infoTimer = 3.0f;
        std::cout << "조준점 색: " << st.colorName() << "\n";
    }
    if (in.keyPressed('3')) {
        st.cycleShape();
        st.infoTimer = 3.0f;
        std::cout << "조준점 모양: " << st.shapeName() << "\n";
    }
}

void drawWorld(App& app, float bossFlash) {
    Renderer& r = app.renderer();
    const Snapshot& s = app.session().snapshot();

    r.beginFrame(app.camera());
    r.drawRoom();

    // 표적
    for (const TargetState& t : s.targets) {
        r.drawSphere(t.pos, t.radius, t.progress > 0.0f ? palette::kTracked : palette::kTarget);
    }

    // 보스
    const BossState& boss = s.boss;
    if (boss.alive) {
        const glm::vec3 base = boss.shield ? palette::kShield : palette::kBoss;
        const glm::vec3 color = glm::mix(base, palette::kWhite, std::clamp(bossFlash, 0.0f, 1.0f));
        r.drawSphere(boss.pos, boss.radius, color);
    }

    // 파편
    for (const auto& p : app.debris().pieces()) {
        glm::mat4 m = glm::translate(glm::mat4(1.0f), p.pos);
        m = glm::rotate(m, glm::radians(p.rot.x), glm::vec3(1, 0, 0));
        m = glm::rotate(m, glm::radians(p.rot.y), glm::vec3(0, 1, 0));
        m = glm::rotate(m, glm::radians(p.rot.z), glm::vec3(0, 0, 1));
        m = glm::scale(m, glm::vec3(p.size));
        r.drawCube(m, p.color);
    }

    // 따라가는 표적 위 타이머 바
    for (const TargetState& t : s.targets) {
        if (t.progress <= 0.0f) continue;
        const float done = std::min(1.0f, t.progress / cfg::kTrackHoldTime);
        const glm::vec3 pos = t.pos + glm::vec3(0.0f, t.radius + 0.3f, 0.0f);
        r.drawBar3D(pos, t.radius * 2.0f, 0.1f, 1.0f - done, glm::vec3(done, 1.0f - done, 0.0f));
    }

    r.drawGunFirstPerson(palette::kGun);
}

void drawCrosshair(App& app) {
    const Settings& st = app.settings();
    app.renderer().drawCrosshair(st.shape == CrosshairShape::Circle, st.colorVec());
}

void drawInfoOverlay(App& app) {
    const Settings& st = app.settings();
    if (st.infoTimer <= 0.0f) return;
    Renderer& r = app.renderer();
    const float cx = r.width() / 2.0f;
    const float y = r.height() / 2.0f + 120.0f;
    r.drawTextCentered(cx, y, format("Crosshair Color: %s", st.colorName()), palette::kWhite);
    r.drawTextCentered(cx, y - 30.0f, format("Crosshair Shape: %s", st.shapeName()), palette::kWhite);
}

void drawHint(App& app, const std::string& text) {
    Renderer& r = app.renderer();
    r.drawRect(0.0f, 0.0f, (float)r.width(), 30.0f, glm::vec4(0.0f, 0.0f, 0.0f, 0.5f));
    r.drawTextCentered(r.width() / 2.0f, 10.0f, text, palette::kGray, Font::Small);
}

void drawPanel(App& app, float x, float y, float w, float h, float alpha) {
    app.renderer().drawRect(x, y, w, h, glm::vec4(0.0f, 0.0f, 0.0f, alpha));
}

} // namespace scene
