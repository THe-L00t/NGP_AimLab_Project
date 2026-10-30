#pragma once
// 화면에 쓰는 색 모음
#include <glm/glm.hpp>

namespace palette {

const glm::vec3 kTarget(0.0f, 0.5f, 1.0f);          // 원본 표적 색 (파랑)
const glm::vec3 kTracked(1.0f, 0.0f, 0.0f);         // 따라가는 중인 표적 (원본: 빨강)
const glm::vec3 kBoss(0.85f, 0.15f, 0.6f);
const glm::vec3 kShield(0.3f, 0.9f, 1.0f);          // 보호막 중인 보스 (따라가야 함)
const glm::vec3 kGun(0.3f, 0.3f, 0.3f);             // 원본 총 색 (회색, 텍스처가 없을 때)

const glm::vec3 kWhite(1.0f, 1.0f, 1.0f);
const glm::vec3 kGray(0.7f, 0.7f, 0.7f);
const glm::vec3 kYellow(1.0f, 0.85f, 0.3f);
const glm::vec3 kGreen(0.45f, 1.0f, 0.45f);
const glm::vec3 kRed(1.0f, 0.4f, 0.4f);

} // namespace palette
