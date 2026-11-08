#pragma once
// 명중 파편 효과 (그리기는 Renderer)
#include <glm/glm.hpp>
#include <random>
#include <vector>

class DebrisSystem {
public:
    struct Piece {
        glm::vec3 pos;
        glm::vec3 vel;
        glm::vec3 rot;      // 도
        glm::vec3 spin;     // 도/초
        glm::vec3 color;
        float size;
        float life;         // 초
    };

    DebrisSystem() : rng_(std::random_device{}()) {}

    void targetBurst(const glm::vec3& pos, const glm::vec3& color);   // 4조각
    void burst(const glm::vec3& pos, const glm::vec3& color, int count, float size, float speed);
    void update(float dt, float floorY);
    void clear() { pieces_.clear(); }
    const std::vector<Piece>& pieces() const { return pieces_; }

private:
    float randf(float lo, float hi) { return std::uniform_real_distribution<float>(lo, hi)(rng_); }
    void add(const glm::vec3& pos, const glm::vec3& vel, const glm::vec3& color, float size);

    std::vector<Piece> pieces_;
    std::mt19937 rng_;
};
