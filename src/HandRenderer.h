#ifndef HANDRENDERER_H
#define HANDRENDERER_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <vector>

class Shader;

class HandRenderer {
public:
    HandRenderer();
    ~HandRenderer();

    void init();
    void cleanup();

    void update(float dt, bool moving, bool jumping, bool movingBackward);
    void render(Shader& shader, float aspect);

private:
    unsigned int vao, vbo;
    int vertexCount;

    float walkCycle;     // 0 to 2π for walking animation
    float swingAmount;   // current swing offset
    float bobOffset;     // vertical bob
    float jumpOffset;    // raise when jumping

    void buildHandMesh();

    // Push a colored cube face
    void pushVertex(std::vector<float>& v,
                    float px, float py, float pz,
                    float r, float g, float b,
                    float nx, float ny, float nz);
    void pushQuad(std::vector<float>& v,
                  float x0, float y0, float z0,
                  float x1, float y1, float z1,
                  float x2, float y2, float z2,
                  float x3, float y3, float z3,
                  float r, float g, float b,
                  float nx, float ny, float nz);
    void addCube(std::vector<float>& v,
                 float x, float y, float z,
                 float sx, float sy, float sz,
                 float r, float g, float b);
};

#endif
