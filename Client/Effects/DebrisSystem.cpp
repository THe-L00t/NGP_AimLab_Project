#include "Effects/DebrisSystem.h"
#include <cmath>

namespace {
constexpr float kLifetime = 3.0f;
constexpr float kGravity = 9.8f;
constexpr float kPi = 3.14159265358979f;
}

void DebrisSystem::add(const glm::vec3& pos, const glm::vec3& vel, const glm::vec3& color, float size) {
    Piece p;
    p.pos = pos;
    p.vel = vel;
    p.rot = glm::vec3(0.0f);
    p.spin = glm::vec3(randf(-10.0f, 10.0f), randf(-10.0f, 10.0f), randf(-10.0f, 10.0f)) * 10.0f;
    p.color = color;
    p.size = size;
    p.life = kLifetime;
    pieces_.push_back(p);
}

void DebrisSystem::targetBurst(const glm::vec3& pos, const glm::vec3& color) {
    const float spread = 2.0f;
    const float size = 0.3f;
    add(pos, glm::vec3(-spread, spread * 2.0f, randf(-1.0f, 1.0f)), color, size);
    add(pos, glm::vec3(spread, spread * 2.0f, randf(-1.0f, 1.0f)), color, size);
    add(pos, glm::vec3(randf(-1.0f, 1.0f), spread * 1.5f, spread), color, size);
    add(pos, glm::vec3(randf(-1.0f, 1.0f), spread * 1.5f, -spread), color, size);
}

void DebrisSystem::burst(const glm::vec3& pos, const glm::vec3& color, int count, float size, float speed) {
    for (int i = 0; i < count; ++i) {
        const float a = randf(0.0f, 2.0f * kPi);
        const glm::vec3 vel(std::cos(a) * speed, randf(0.5f, 1.5f) * speed, std::sin(a) * speed);
        add(pos, vel, color, size);
    }
}

void DebrisSystem::update(float dt, float floorY) {
    for (auto& p : pieces_) {
        p.life -= dt;
        p.vel.y -= kGravity * dt;
        p.pos += p.vel * dt;
        p.rot += p.spin * dt;
        if (p.pos.y <= floorY) {        // 바닥에서 튕기고 멈춤
            p.pos.y = floorY;
            p.vel.y = -p.vel.y * 0.5f;
            p.vel.x *= 0.8f;
            p.vel.z *= 0.8f;
            if (glm::length(p.vel) < 0.1f) {
                p.vel = glm::vec3(0.0f);
                p.spin *= 0.5f;
            }
        }
    }
    size_t w = 0;
    for (size_t r = 0; r < pieces_.size(); ++r) {
        if (pieces_[r].life > 0.0f) pieces_[w++] = pieces_[r];
    }
    pieces_.resize(w);
}
