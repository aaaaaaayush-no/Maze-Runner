#include "Renderer.h"
#include "Maze.h"
#include "Collectible.h"
#include "Shader.h"
#include <glm/gtc/matrix_transform.hpp>
#include <cmath>

static const float CELL_SIZE = 2.0f;
static const float WALL_HEIGHT = 4.0f;

Renderer::Renderer()
    : mazeVAO(0), mazeVBO(0), mazeVertexCount(0)
    , cubeVAO(0), cubeVBO(0), cubeVertexCount(0)
    , pyramidVAO(0), pyramidVBO(0), pyramidVertexCount(0)
    , sphereVAO(0), sphereVBO(0), sphereVertexCount(0)
    , wireframe(false)
{
}

Renderer::~Renderer() {
    if (mazeVAO) { glDeleteVertexArrays(1, &mazeVAO); glDeleteBuffers(1, &mazeVBO); }
    if (cubeVAO) { glDeleteVertexArrays(1, &cubeVAO); glDeleteBuffers(1, &cubeVBO); }
    if (pyramidVAO) { glDeleteVertexArrays(1, &pyramidVAO); glDeleteBuffers(1, &pyramidVBO); }
    if (sphereVAO) { glDeleteVertexArrays(1, &sphereVAO); glDeleteBuffers(1, &sphereVBO); }
}

void Renderer::init() {
    buildCubeMesh();
    buildPyramidMesh();
    buildSphereMesh();
}

void Renderer::setWireframe(bool enabled) {
    wireframe = enabled;
    glPolygonMode(GL_FRONT_AND_BACK, wireframe ? GL_LINE : GL_FILL);
}

// Helper: add a vertex (pos + color + normal)
static void pushVertex(std::vector<float>& v,
                       float px, float py, float pz,
                       float r, float g, float b,
                       float nx, float ny, float nz) {
    v.insert(v.end(), {px, py, pz, r, g, b, nx, ny, nz});
}

// Helper: add a quad (two triangles)
static void pushQuad(std::vector<float>& v,
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

void Renderer::addCube(std::vector<float>& verts,
                       float x, float y, float z,
                       float sx, float sy, float sz,
                       float r, float g, float b) {
    float x0 = x, x1 = x + sx;
    float y0 = y, y1 = y + sy;
    float z0 = z, z1 = z + sz;

    // Front  (+Z)
    pushQuad(verts, x0,y0,z1, x1,y0,z1, x1,y1,z1, x0,y1,z1, r,g,b, 0,0,1);
    // Back   (-Z)
    pushQuad(verts, x1,y0,z0, x0,y0,z0, x0,y1,z0, x1,y1,z0, r,g,b, 0,0,-1);
    // Left   (-X)
    pushQuad(verts, x0,y0,z0, x0,y0,z1, x0,y1,z1, x0,y1,z0, r,g,b, -1,0,0);
    // Right  (+X)
    pushQuad(verts, x1,y0,z1, x1,y0,z0, x1,y1,z0, x1,y1,z1, r,g,b, 1,0,0);
    // Top    (+Y)
    pushQuad(verts, x0,y1,z1, x1,y1,z1, x1,y1,z0, x0,y1,z0, r,g,b, 0,1,0);
    // Bottom (-Y)
    pushQuad(verts, x0,y0,z0, x1,y0,z0, x1,y0,z1, x0,y0,z1, r,g,b, 0,-1,0);
}

void Renderer::buildMazeMesh(const Maze& maze) {
    std::vector<float> verts;

    int w = maze.getWidth();
    int h = maze.getHeight();

    for (int y = 0; y < h; y++) {
        for (int x = 0; x < w; x++) {
            float wx = x * CELL_SIZE;
            float wz = y * CELL_SIZE;

            if (maze.isWall(x, y)) {
                // Wall cube
                addCube(verts, wx, 0.0f, wz, CELL_SIZE, WALL_HEIGHT, CELL_SIZE,
                        0.45f, 0.40f, 0.35f); // gray-brown walls
            } else {
                // Floor
                pushQuad(verts,
                    wx,         0.0f, wz,
                    wx+CELL_SIZE, 0.0f, wz,
                    wx+CELL_SIZE, 0.0f, wz+CELL_SIZE,
                    wx,         0.0f, wz+CELL_SIZE,
                    0.2f, 0.2f, 0.22f, // dark gray floor
                    0.0f, 1.0f, 0.0f);
                // Ceiling
                pushQuad(verts,
                    wx,         WALL_HEIGHT, wz+CELL_SIZE,
                    wx+CELL_SIZE, WALL_HEIGHT, wz+CELL_SIZE,
                    wx+CELL_SIZE, WALL_HEIGHT, wz,
                    wx,         WALL_HEIGHT, wz,
                    0.05f, 0.05f, 0.1f, // dark blue ceiling
                    0.0f, -1.0f, 0.0f);
            }
        }
    }

    mazeVertexCount = (int)(verts.size() / 9);

    if (mazeVAO) { glDeleteVertexArrays(1, &mazeVAO); glDeleteBuffers(1, &mazeVBO); }

    glGenVertexArrays(1, &mazeVAO);
    glGenBuffers(1, &mazeVBO);

    glBindVertexArray(mazeVAO);
    glBindBuffer(GL_ARRAY_BUFFER, mazeVBO);
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

void Renderer::renderMaze(Shader& shader, const glm::mat4& view, const glm::mat4& projection) {
    shader.use();
    glm::mat4 model = glm::mat4(1.0f);
    shader.setMat4("model", model);
    shader.setMat4("view", view);
    shader.setMat4("projection", projection);
    shader.setVec3("lightDir", glm::vec3(0.3f, 1.0f, 0.5f));
    shader.setVec3("fogColor", glm::vec3(0.05f, 0.05f, 0.08f));
    shader.setFloat("fogDensity", 0.04f);
    shader.setFloat("fogGradient", 2.5f);

    glBindVertexArray(mazeVAO);
    glDrawArrays(GL_TRIANGLES, 0, mazeVertexCount);
    glBindVertexArray(0);
}

void Renderer::buildCubeMesh() {
    std::vector<float> verts;
    addCube(verts, -0.5f, -0.5f, -0.5f, 1.0f, 1.0f, 1.0f, 1.0f, 0.84f, 0.0f);
    cubeVertexCount = (int)(verts.size() / 9);

    glGenVertexArrays(1, &cubeVAO);
    glGenBuffers(1, &cubeVBO);
    glBindVertexArray(cubeVAO);
    glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
    glBufferData(GL_ARRAY_BUFFER, verts.size() * sizeof(float), verts.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 9*sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 9*sizeof(float), (void*)(3*sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 9*sizeof(float), (void*)(6*sizeof(float)));
    glEnableVertexAttribArray(2);
    glBindVertexArray(0);
}

void Renderer::buildPyramidMesh() {
    std::vector<float> verts;
    // Base quad
    float r = 0.6f, g = 0.2f, b = 0.8f; // purple
    float s = 0.5f, h = 1.0f;
    // 4 triangular faces
    // Front
    pushVertex(verts, 0,h,0, r,g,b, 0,0.5f,1); pushVertex(verts, -s,0,s, r,g,b, 0,0.5f,1); pushVertex(verts, s,0,s, r,g,b, 0,0.5f,1);
    // Right
    pushVertex(verts, 0,h,0, r,g,b, 1,0.5f,0); pushVertex(verts, s,0,s, r,g,b, 1,0.5f,0); pushVertex(verts, s,0,-s, r,g,b, 1,0.5f,0);
    // Back
    pushVertex(verts, 0,h,0, r,g,b, 0,0.5f,-1); pushVertex(verts, s,0,-s, r,g,b, 0,0.5f,-1); pushVertex(verts, -s,0,-s, r,g,b, 0,0.5f,-1);
    // Left
    pushVertex(verts, 0,h,0, r,g,b, -1,0.5f,0); pushVertex(verts, -s,0,-s, r,g,b, -1,0.5f,0); pushVertex(verts, -s,0,s, r,g,b, -1,0.5f,0);
    // Base
    pushVertex(verts, -s,0,s, r,g,b, 0,-1,0); pushVertex(verts, s,0,s, r,g,b, 0,-1,0); pushVertex(verts, s,0,-s, r,g,b, 0,-1,0);
    pushVertex(verts, -s,0,s, r,g,b, 0,-1,0); pushVertex(verts, s,0,-s, r,g,b, 0,-1,0); pushVertex(verts, -s,0,-s, r,g,b, 0,-1,0);

    pyramidVertexCount = (int)(verts.size() / 9);

    glGenVertexArrays(1, &pyramidVAO);
    glGenBuffers(1, &pyramidVBO);
    glBindVertexArray(pyramidVAO);
    glBindBuffer(GL_ARRAY_BUFFER, pyramidVBO);
    glBufferData(GL_ARRAY_BUFFER, verts.size() * sizeof(float), verts.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 9*sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 9*sizeof(float), (void*)(3*sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 9*sizeof(float), (void*)(6*sizeof(float)));
    glEnableVertexAttribArray(2);
    glBindVertexArray(0);
}

void Renderer::buildSphereMesh() {
    std::vector<float> verts;
    float r = 0.3f, g = 0.8f, b = 1.0f; // cyan-ish
    int stacks = 8, slices = 12;
    float radius = 0.4f;

    for (int i = 0; i < stacks; i++) {
        float phi0 = glm::pi<float>() * (float)i / stacks - glm::half_pi<float>();
        float phi1 = glm::pi<float>() * (float)(i + 1) / stacks - glm::half_pi<float>();
        for (int j = 0; j < slices; j++) {
            float theta0 = 2.0f * glm::pi<float>() * (float)j / slices;
            float theta1 = 2.0f * glm::pi<float>() * (float)(j + 1) / slices;

            auto spherePos = [&](float phi, float theta) -> glm::vec3 {
                return glm::vec3(
                    radius * cos(phi) * cos(theta),
                    radius * sin(phi),
                    radius * cos(phi) * sin(theta)
                );
            };

            glm::vec3 p0 = spherePos(phi0, theta0);
            glm::vec3 p1 = spherePos(phi0, theta1);
            glm::vec3 p2 = spherePos(phi1, theta1);
            glm::vec3 p3 = spherePos(phi1, theta0);

            glm::vec3 n0 = glm::normalize(p0);
            glm::vec3 n1 = glm::normalize(p1);
            glm::vec3 n2 = glm::normalize(p2);
            glm::vec3 n3 = glm::normalize(p3);

            pushVertex(verts, p0.x,p0.y,p0.z, r,g,b, n0.x,n0.y,n0.z);
            pushVertex(verts, p1.x,p1.y,p1.z, r,g,b, n1.x,n1.y,n1.z);
            pushVertex(verts, p2.x,p2.y,p2.z, r,g,b, n2.x,n2.y,n2.z);

            pushVertex(verts, p0.x,p0.y,p0.z, r,g,b, n0.x,n0.y,n0.z);
            pushVertex(verts, p2.x,p2.y,p2.z, r,g,b, n2.x,n2.y,n2.z);
            pushVertex(verts, p3.x,p3.y,p3.z, r,g,b, n3.x,n3.y,n3.z);
        }
    }

    sphereVertexCount = (int)(verts.size() / 9);

    glGenVertexArrays(1, &sphereVAO);
    glGenBuffers(1, &sphereVBO);
    glBindVertexArray(sphereVAO);
    glBindBuffer(GL_ARRAY_BUFFER, sphereVBO);
    glBufferData(GL_ARRAY_BUFFER, verts.size() * sizeof(float), verts.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 9*sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 9*sizeof(float), (void*)(3*sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 9*sizeof(float), (void*)(6*sizeof(float)));
    glEnableVertexAttribArray(2);
    glBindVertexArray(0);
}

void Renderer::renderCollectibles(Shader& shader, const glm::mat4& view,
                                  const glm::mat4& projection,
                                  const std::vector<CollectibleItem>& items) {
    shader.use();
    shader.setMat4("view", view);
    shader.setMat4("projection", projection);
    shader.setVec3("lightDir", glm::vec3(0.3f, 1.0f, 0.5f));
    shader.setVec3("fogColor", glm::vec3(0.05f, 0.05f, 0.08f));
    shader.setFloat("fogDensity", 0.04f);
    shader.setFloat("fogGradient", 2.5f);

    for (auto& item : items) {
        if (item.collected) continue;

        glm::mat4 model = glm::translate(glm::mat4(1.0f), item.position);
        model = glm::rotate(model, glm::radians(item.rotationAngle), glm::vec3(0, 1, 0));

        float bob = sin(item.rotationAngle * 0.03f) * 0.15f;
        model = glm::translate(model, glm::vec3(0, bob, 0));
        model = glm::scale(model, glm::vec3(0.5f));

        shader.setMat4("model", model);

        switch (item.type) {
            case ItemType::KEY:
                glBindVertexArray(cubeVAO);
                glDrawArrays(GL_TRIANGLES, 0, cubeVertexCount);
                break;
            case ItemType::ARTIFACT:
                glBindVertexArray(pyramidVAO);
                glDrawArrays(GL_TRIANGLES, 0, pyramidVertexCount);
                break;
            case ItemType::ORB:
                glBindVertexArray(sphereVAO);
                glDrawArrays(GL_TRIANGLES, 0, sphereVertexCount);
                break;
        }
        glBindVertexArray(0);
    }
}

void Renderer::renderExitPortal(Shader& shader, const glm::mat4& view,
                                const glm::mat4& projection,
                                const glm::vec3& exitPos, float time) {
    shader.use();
    shader.setMat4("view", view);
    shader.setMat4("projection", projection);
    shader.setVec3("lightDir", glm::vec3(0.3f, 1.0f, 0.5f));
    shader.setVec3("fogColor", glm::vec3(0.05f, 0.05f, 0.08f));
    shader.setFloat("fogDensity", 0.04f);
    shader.setFloat("fogGradient", 2.5f);

    // Pulsing green portal
    float pulse = 0.7f + 0.3f * sin(time * 3.0f);
    glm::mat4 model = glm::translate(glm::mat4(1.0f), exitPos);
    model = glm::rotate(model, time * 1.5f, glm::vec3(0, 1, 0));
    model = glm::scale(model, glm::vec3(0.8f * pulse, 1.5f * pulse, 0.8f * pulse));
    shader.setMat4("model", model);

    // Render the sphere mesh scaled up as the pulsing portal indicator.
    glBindVertexArray(sphereVAO);
    glDrawArrays(GL_TRIANGLES, 0, sphereVertexCount);
    glBindVertexArray(0);
}
