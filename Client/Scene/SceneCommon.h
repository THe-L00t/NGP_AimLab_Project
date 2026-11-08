#pragma once
// 씬 공용 처리
#include <glm/glm.hpp>
#include <string>

class App;

namespace scene {

// 마우스 → 카메라 → 시선을 로직에 알림
void updateLook(App& app);

// Q 종료, M 정보, allowSettings면 2·3 조준점
void handleCommonKeys(App& app, bool allowSettings);

// 방·표적·보스·파편·총 (bossFlash 0~1)
void drawWorld(App& app, float bossFlash = 0.0f);

// 2D UI
void drawCrosshair(App& app);
void drawInfoOverlay(App& app);
void drawHint(App& app, const std::string& text);
void drawPanel(App& app, float x, float y, float w, float h, float alpha = 0.55f);

std::string format(const char* fmt, ...);

} // namespace scene
