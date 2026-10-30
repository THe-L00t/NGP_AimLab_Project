#pragma once
// 총 모델. Resources/Gun.obj (+ Gun.mtl의 map_Kd 텍스처)가 있으면 그것을, 없으면 상자 3개로 만든 임시 모양을 쓴다.
//
// OBJ의 크기·방향은 자동으로 맞춘다 (다른 모델로 바꿔도 그대로 동작하게):
//   - Y가 위. X와 Z 중 더 긴 축을 총열 방향으로 본다.
//   - 아래쪽 30% 정점(손잡이)이 몰린 쪽이 뒤, 그 반대쪽이 총구.
//   - 총구가 눈 좌표계 정면(-Z)을 향하게 돌리고, 길이를 kGunLength에 맞춰 화면 오른쪽 아래(kGunPosition)에 둔다.
#include "Render/Mesh.h"
#include "Render/Texture.h"
#include <glm/glm.hpp>
#include <string>

class GunModel {
public:
    void load(const std::string& objPath);
    void destroy() { mesh_.destroy(); texture_.destroy(); }

    const Mesh& mesh() const { return mesh_; }
    // 눈(카메라) 좌표계 기준 총의 위치·방향 (1인칭으로 그릴 때 이 행렬 그대로 쓴다)
    const glm::mat4& viewTransform() const { return viewTransform_; }
    bool fromFile() const { return fromFile_; }
    const Texture& texture() const { return texture_; }
    bool hasTexture() const { return texture_.valid(); }

private:
    bool loadObj(const std::string& path);
    void loadMaterialTexture(const std::string& objPath, const std::string& mtlName);
    void buildFallback();

    Mesh mesh_;
    Texture texture_;
    glm::mat4 viewTransform_{ 1.0f };
    bool fromFile_ = false;
};
