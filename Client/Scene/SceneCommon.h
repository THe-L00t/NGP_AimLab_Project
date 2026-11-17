#pragma once
// 씬 공용 처리
#include <glm/glm.hpp>
#include <string>

class App;

namespace scene {

// 마우스 → 카메라 → 시선을 로직에 알림
void updateLook(App& app);

// 자리의 눈 위치. 서버가 모든 자리를 자리 0 위치에서 판정하므로(World::fire) 지금은 모두 같다
glm::vec3 seatEye(int slot);
int mySlot(App& app);         // 내 자리 (1인 모드·입장 전은 0)
glm::vec3 myEye(App& app);    // 내 자리의 눈 위치

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
