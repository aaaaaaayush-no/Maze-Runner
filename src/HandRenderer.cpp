#include "HandRenderer.h"
#include "Shader.h"
#include <glm/gtc/matrix_transform.hpp>
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

HandRenderer::HandRenderer()
    : vao(0), vbo(0), vertexCount(0)
    , walkCycle(0.0f), swingAmount(0.0f)
    , bobOffset(0.0f), jumpOffset(0.0f)
{
}

HandRenderer::~HandRenderer() {
    cleanup();
}

void HandRenderer::init() {
    buildHandMesh();
}

void HandRenderer::cleanup() {
    if (vao) { glDeleteVertexArrays(1, &vao); vao = 0; }
    if (vbo) { glDeleteBuffers(1, &vbo); vbo = 0; }
}

void HandRenderer::pushVertex(std::vector<float>& v,
                               float px, float py, float pz,
                               float r, float g, float b,
                               float nx, float ny, float nz) {
    v.insert(v.end(), {px, py, pz, r, g, b, nx, ny, nz});
}

void HandRenderer::pushQuad(std::vector<float>& v,
                             float x0, float y0, float z0,
                             float x1, float y1, float z1,
                             float x2, float y2, float z2,
                             float x3, float y3, float z3,
                             float r, float g, float b,
                             float nx, float ny, float nz) {
    pushVertex(v, x0,y0,z0, r,g,b, nx,ny,nz);
    pushVertex(v, x1,y1,z1, r,g,b, nx,ny,nz);
    pushVertex(v, x2,y2,z2, r,g,b, nx,ny,nz);
    pushVertex(v, x2,y2,z2, r,g,b, nx,ny,nz);
    pushVertex(v, x3,y3,z3, r,g,b, nx,ny,nz);
    pushVertex(v, x0,y0,z0, r,g,b, nx,ny,nz);
}

void HandRenderer::addCube(std::vector<float>& verts,
                           float x, float y, float z,
                           float sx, float sy, float sz,
                           float r, float g, float b) {
    float x0 = x, x1 = x + sx;
    float y0 = y, y1 = y + sy;
    float z0 = z, z1 = z + sz;

    // Front  (+Z)
    pushQuad(verts, x0,y0,z1, x1,y0,z1, x1,y1,z1, x0,y1,z1, r,g,b, 0,0,1);
    // Back   (-Z)
    pushQuad(verts, x1,y0,z0, x0,y0,z0, x0,y1,z0, x1,y1,z0, r*0.7f,g*0.7f,b*0.7f, 0,0,-1);
    // Left   (-X)
    pushQuad(verts, x0,y0,z0, x0,y0,z1, x0,y1,z1, x0,y1,z0, r*0.85f,g*0.85f,b*0.85f, -1,0,0);
    // Right  (+X)
    pushQuad(verts, x1,y0,z1, x1,y0,z0, x1,y1,z0, x1,y1,z1, r*0.85f,g*0.85f,b*0.85f, 1,0,0);
    // Top    (+Y)
    pushQuad(verts, x0,y1,z1, x1,y1,z1, x1,y1,z0, x0,y1,z0, r*1.0f,g*1.0f,b*1.0f, 0,1,0);
    // Bottom (-Y)
    pushQuad(verts, x0,y0,z0, x1,y0,z0, x1,y0,z1, x0,y0,z1, r*0.6f,g*0.6f,b*0.6f, 0,-1,0);
}

void HandRenderer::buildHandMesh() {
    std::vector<float> verts;

    // Steve-style hand: a rectangular cuboid
    // Skin color (Steve's skin tone)
    float skinR = 0.72f, skinG = 0.53f, skinB = 0.38f;
    // Shirt sleeve color (cyan/teal like Steve's shirt)
    float sleeveR = 0.25f, sleeveG = 0.65f, sleeveB = 0.65f;

    // Hand (forearm + hand part)
    // The hand mesh is centered so we can position it with transforms
    // Forearm/sleeve part (upper portion)
    addCube(verts, -0.15f, -0.15f, -0.6f, 0.30f, 0.30f, 0.35f,
            sleeveR, sleeveG, sleeveB);

    // Hand/skin part (lower portion, closer to camera)
    addCube(verts, -0.15f, -0.15f, -0.25f, 0.30f, 0.30f, 0.25f,
            skinR, skinG, skinB);

    vertexCount = (int)(verts.size() / 9);

    if (vao) { glDeleteVertexArrays(1, &vao); glDeleteBuffers(1, &vbo); }

    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, verts.size() * sizeof(float), verts.data(), GL_STATIC_DRAW);

    // Position
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // Color
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    // Normal
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    glBindVertexArray(0);
}

void HandRenderer::update(float dt, bool moving, bool jumping, bool movingBackward) {
    float targetBob = 0.0f;
    float targetSwing = 0.0f;

    if (moving) {
        walkCycle += dt * 8.0f; // walking speed
        if (walkCycle > 2.0f * (float)M_PI) walkCycle -= 2.0f * (float)M_PI;

        targetBob = std::sin(walkCycle) * 0.03f;
        targetSwing = std::sin(walkCycle) * 0.08f;

        if (movingBackward) {
            targetSwing = -targetSwing * 0.5f;
        }
    } else {
        // Gentle idle sway
        walkCycle += dt * 1.5f;
        if (walkCycle > 2.0f * (float)M_PI) walkCycle -= 2.0f * (float)M_PI;
        targetBob = std::sin(walkCycle) * 0.005f;
        targetSwing = 0.0f;
    }

    // Jump offset
    float targetJump = jumping ? 0.05f : 0.0f;

    // Smooth interpolation
    float lerpSpeed = 10.0f * dt;
    bobOffset += (targetBob - bobOffset) * lerpSpeed;
    swingAmount += (targetSwing - swingAmount) * lerpSpeed;
    jumpOffset += (targetJump - jumpOffset) * lerpSpeed;
}

void HandRenderer::render(Shader& shader, float aspect) {
    if (!vao) return;

    shader.use();

    // Separate projection for hands (smaller FOV to avoid distortion)
    glm::mat4 handProj = glm::perspective(glm::radians(70.0f), aspect, 0.01f, 10.0f);
    glm::mat4 handView = glm::mat4(1.0f); // identity - hands are in view space

    shader.setMat4("view", handView);
    shader.setMat4("projection", handProj);
    shader.setVec3("lightDir", glm::vec3(0.0f, 1.0f, 0.5f));
    shader.setVec3("fogColor", glm::vec3(0.05f, 0.05f, 0.08f));
    shader.setFloat("fogDensity", 0.0f); // no fog on hands
    shader.setFloat("fogGradient", 1.0f);

    // Render hands with separate depth range so they're always on top
    glDepthRange(0.0, 0.1);

    // Right hand
    {
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.45f, -0.45f + bobOffset + jumpOffset, -0.6f));
        model = glm::rotate(model, glm::radians(-15.0f + swingAmount * 20.0f), glm::vec3(1, 0, 0));
        model = glm::rotate(model, glm::radians(10.0f), glm::vec3(0, 1, 0));
        model = glm::scale(model, glm::vec3(0.5f));
        shader.setMat4("model", model);

        glBindVertexArray(vao);
        glDrawArrays(GL_TRIANGLES, 0, vertexCount);
    }

    // Left hand
    {
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(-0.45f, -0.45f + bobOffset + jumpOffset, -0.6f));
        model = glm::rotate(model, glm::radians(-15.0f - swingAmount * 20.0f), glm::vec3(1, 0, 0));
        model = glm::rotate(model, glm::radians(-10.0f), glm::vec3(0, 1, 0));
        model = glm::scale(model, glm::vec3(0.5f));
        shader.setMat4("model", model);

        glBindVertexArray(vao);
        glDrawArrays(GL_TRIANGLES, 0, vertexCount);
    }

    glBindVertexArray(0);

    // Restore default depth range
    glDepthRange(0.0, 1.0);
}
