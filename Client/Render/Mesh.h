#pragma once
// VAO/VBO/EBO 묶음 + 기본 도형 생성
#include "Render/GL.h"
#include <vector>

class Mesh {
public:
    // stride: 3 위치, 6 +법선, 8 +UV
    void create(const std::vector<float>& vertices, int stride,
                const std::vector<unsigned>& indices, GLenum primitive);
    void destroy();
    void draw() const;
    bool valid() const { return vao_ != 0; }

private:
    GLuint vao_ = 0, vbo_ = 0, ebo_ = 0;
    GLsizei count_ = 0;
    GLenum primitive_ = GL_TRIANGLES;
    bool indexed_ = false;
};

// 정점 데이터만 만든다 (업로드는 Mesh::create)
namespace MeshGen {
void sphere(int sectors, int stacks, std::vector<float>& v, std::vector<unsigned>& i);   // 반지름 1
void cube(std::vector<float>& v, std::vector<unsigned>& i);                               // 한 변 1
void room(float w, float h, float d, float floorY, std::vector<float>& v);                // 안쪽을 보는 상자
void box(const float center[3], const float size[3], std::vector<float>& v, std::vector<unsigned>& i);
void quad01(std::vector<float>& v);     // (0,0)~(1,1), 위치만
void cross(std::vector<float>& v);      // GL_LINES
void circle(int segments, std::vector<float>& v);  // GL_LINE_LOOP
}
