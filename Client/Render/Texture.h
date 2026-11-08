#pragma once
// 2D 텍스처 (stb_image, 밉맵 포함)
#include "Render/GL.h"
#include <string>

class Texture {
public:
    Texture() = default;
    ~Texture() { destroy(); }
    Texture(const Texture&) = delete;
    Texture& operator=(const Texture&) = delete;

    bool loadFromFile(const std::string& path);
    void destroy();
    void bind(int unit) const;
    bool valid() const { return id_ != 0; }

private:
    GLuint id_ = 0;
};
