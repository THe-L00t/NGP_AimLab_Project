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

namespace {
constexpr float kAimMarkerDistance = cfg::kEyeDistance;   // 눈에서 맵 중앙까지
constexpr float kAimMarkerRadius = 0.15f;
const glm::vec3 kPlayerColor[MAX_PLAYERS] = {             // 자리별 시선 점 색
    { 1.0f, 0.55f, 0.1f }, { 0.4f, 1.0f, 0.3f }, { 0.85f, 0.45f, 1.0f }
};
} // namespace

namespace scene {

std::string format(const char* fmt, ...) {
    char buf[256];
    va_list args;
    va_start(args, fmt);
    std::vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);
    return buf;
}

glm::vec3 seatEye(int /*slot*/) {
    return aim::eyePosition();
}

int mySlot(App& app) {
    const NetSession* net = app.net();
    return net && net->slot() >= 0 ? net->slot() : 0;
}

glm::vec3 myEye(App& app) {
    return seatEye(mySlot(app));
}

void updateLook(App& app) {
    const glm::vec2 d = app.input().lookDelta();
    Camera& cam = app.camera();
    if (d.x != 0.0f || d.y != 0.0f) cam.addLook(d.x, d.y);
    cam.setPosition(myEye(app));   // 내 자리에서 본다
    if (!app.net()) app.world().setAim(cam.yaw(), cam.pitch());
    // 네트워크 모드: 시선 전송 (안병규 11/17)
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

    // 다른 플레이어 시선: 그 사람이 보는 방향의 점 (네트워크 모드, 스냅샷의 yaw·pitch)
    if (const NetSession* net = app.net()) {
        for (int i = 0; i < MAX_PLAYERS; ++i) {
            const PlayerState& p = s.players[i];
            if (!p.joined || i == net->slot()) continue;
            const glm::vec3 point = seatEye(i) + aim::direction(p.yaw, p.pitch) * kAimMarkerDistance;
            r.drawSphere(point, kAimMarkerRadius, kPlayerColor[i]);
        }
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
