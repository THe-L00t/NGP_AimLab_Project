#pragma once
// 총 모델: Resources/Gun.obj, 없으면 상자 3개. 크기·방향은 자동으로 맞춘다.
#include "Render/Mesh.h"
#include "Render/Texture.h"
#include <glm/glm.hpp>
#include <string>

class GunModel {
public:
    void load(const std::string& objPath);
    void destroy() { mesh_.destroy(); texture_.destroy(); }

    const Mesh& mesh() const { return mesh_; }
    // 눈 좌표계 기준
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
