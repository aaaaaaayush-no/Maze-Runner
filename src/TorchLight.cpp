#include "TorchLight.h"
#include "Shader.h"
#include <glm/gtc/matrix_transform.hpp>
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

TorchLight::TorchLight()
    : enabled(false)
    , position(0.0f)
    , baseColor(1.0f, 0.7f, 0.3f)  // orange-yellow
    , radius(12.0f)
    , flickerTimer(0.0f)
    , flickerIntensity(1.0f)
    , glowVAO(0), glowVBO(0), glowVertexCount(0)
{
}

void TorchLight::init() {
    buildGlowMesh();
}

void TorchLight::cleanup() {
    if (glowVAO) { glDeleteVertexArrays(1, &glowVAO); glowVAO = 0; }
    if (glowVBO) { glDeleteBuffers(1, &glowVBO); glowVBO = 0; }
}

void TorchLight::toggle() {
    enabled = !enabled;
}

void TorchLight::update(float dt) {
    if (!enabled) return;

    flickerTimer += dt;

    // Combine multiple sine waves for realistic flicker
    flickerIntensity = 0.85f
        + 0.08f * std::sin(flickerTimer * 13.0f)
        + 0.05f * std::sin(flickerTimer * 23.0f)
        + 0.02f * std::sin(flickerTimer * 37.0f);
}

void TorchLight::setPlayerPosition(const glm::vec3& playerPos, const glm::vec3& playerFront) {
    // Position torch slightly in front of and above the player's head
    position = playerPos + playerFront * 0.5f + glm::vec3(0.0f, 0.3f, 0.0f);
}

glm::vec3 TorchLight::getColor() const {
    if (!enabled) return glm::vec3(0.0f);
    return baseColor * flickerIntensity;
}

float TorchLight::getIntensity() const {
    if (!enabled) return 0.0f;
    return flickerIntensity;
}

void TorchLight::buildGlowMesh() {
    // Small diamond-shaped glow sprite
    std::vector<float> verts;
    float s = 0.15f;

    // Vertex format: pos(3) + color(3) + normal(3) for compatibility with main shader
    float r = baseColor.r, g = baseColor.g, b = baseColor.b;

    // Two triangles forming a diamond
    verts.insert(verts.end(), { 0, s, 0, r, g, b, 0, 0, 1});
    verts.insert(verts.end(), {-s, 0, 0, r*0.5f, g*0.5f, b*0.5f, 0, 0, 1});
    verts.insert(verts.end(), { s, 0, 0, r*0.5f, g*0.5f, b*0.5f, 0, 0, 1});

    verts.insert(verts.end(), { 0,-s, 0, r*0.3f, g*0.3f, b*0.3f, 0, 0, 1});
    verts.insert(verts.end(), { s, 0, 0, r*0.5f, g*0.5f, b*0.5f, 0, 0, 1});
    verts.insert(verts.end(), {-s, 0, 0, r*0.5f, g*0.5f, b*0.5f, 0, 0, 1});

    glowVertexCount = 6;

    glGenVertexArrays(1, &glowVAO);
    glGenBuffers(1, &glowVBO);
    glBindVertexArray(glowVAO);
    glBindBuffer(GL_ARRAY_BUFFER, glowVBO);
    glBufferData(GL_ARRAY_BUFFER, verts.size() * sizeof(float), verts.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    glBindVertexArray(0);
}

void TorchLight::renderGlow(Shader& shader, const glm::mat4& view, const glm::mat4& projection) {
    if (!enabled || !glowVAO) return;

    shader.use();
    shader.setMat4("view", view);
    shader.setMat4("projection", projection);
    shader.setBool("useTexture", false);
    shader.setFloat("fogDensity", 0.0f);
    shader.setFloat("fogGradient", 1.0f);
    shader.setBool("torchEnabled", false);
    shader.setBool("enableEdgeOutline", false);
    shader.setVec3("sunColor", glm::vec3(1.0f));
    shader.setFloat("ambientLevel", 1.0f);

    // Billboard the glow sprite toward camera
    glm::mat4 model = glm::translate(glm::mat4(1.0f), position);

    // Extract camera right and up from view matrix
    glm::vec3 camRight(view[0][0], view[1][0], view[2][0]);
    glm::vec3 camUp(view[0][1], view[1][1], view[2][1]);

    // Build billboard rotation
    model[0] = glm::vec4(camRight, 0.0f);
    model[1] = glm::vec4(camUp, 0.0f);
    model[2] = glm::vec4(glm::cross(camRight, camUp), 0.0f);

    float scale = 0.3f * flickerIntensity;
    model = glm::scale(model, glm::vec3(scale));

    shader.setMat4("model", model);

    glDisable(GL_CULL_FACE);
    glBindVertexArray(glowVAO);
    glDrawArrays(GL_TRIANGLES, 0, glowVertexCount);
    glBindVertexArray(0);
    glEnable(GL_CULL_FACE);
}
