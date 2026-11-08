#pragma once
// 조준·명중 계산 (화면과 판정이 같이 쓴다)
#include "GameConfig.h"
#include <glm/glm.hpp>
#include <cmath>

namespace aim {

// yaw 0 = +X, yaw 증가 = +Z 쪽
inline glm::vec3 direction(float yawDeg, float pitchDeg) {
    const float y = glm::radians(yawDeg);
    const float p = glm::radians(pitchDeg);
    return glm::normalize(glm::vec3(std::cos(y) * std::cos(p),
                                    std::sin(p),
                                    std::sin(y) * std::cos(p)));
}

inline float clampPitch(float pitchDeg) {
    if (pitchDeg > 89.0f) return 89.0f;
    if (pitchDeg < -89.0f) return -89.0f;
    return pitchDeg;
}

// 광선-구 교차 (dir 정규화, 등 뒤는 제외)
inline bool raySphere(const glm::vec3& origin, const glm::vec3& dir,
                      const glm::vec3& center, float radius, float& t) {
    const glm::vec3 oc = origin - center;
    const float b = glm::dot(oc, dir);
    const float c = glm::dot(oc, oc) - radius * radius;
    const float disc = b * b - c;
    if (disc < 0.0f) return false;
    const float s = std::sqrt(disc);
    t = -b - s;
    if (t < 0.0f) t = -b + s;   // 구 안에서 쏜 경우
    return t >= 0.0f;
}

// 자리 0의 눈 위치 (맵 중앙의 -X 쪽)
inline glm::vec3 eyePosition() {
    return glm::vec3(-cfg::kEyeDistance, cfg::kEyeHeight, 0.0f);
}

inline glm::vec3 arenaCenter() { return glm::vec3(0.0f, cfg::kArenaCenterY, 0.0f); }

inline void lookAngles(const glm::vec3& eye, const glm::vec3& target, float& yawDeg, float& pitchDeg) {
    const glm::vec3 d = target - eye;
    yawDeg = glm::degrees(std::atan2(d.z, d.x));
    pitchDeg = glm::degrees(std::atan2(d.y, std::sqrt(d.x * d.x + d.z * d.z)));
}

} // namespace aim
