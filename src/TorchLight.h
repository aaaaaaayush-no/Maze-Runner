#ifndef TORCHLIGHT_H
#define TORCHLIGHT_H

#include <glm/glm.hpp>
#include <glad/glad.h>
#include <vector>

class Shader;

class TorchLight {
public:
    TorchLight();

    void init();
    void cleanup();

    void toggle();
    void update(float dt);

    bool isEnabled() const { return enabled; }
    glm::vec3 getPosition() const { return position; }
    glm::vec3 getColor() const;
    float getIntensity() const;
    float getRadius() const { return radius; }

    // Update position based on player
    void setPlayerPosition(const glm::vec3& playerPos, const glm::vec3& playerFront);

    // Render the torch glow sprite (small glowing quad)
    void renderGlow(Shader& shader, const glm::mat4& view, const glm::mat4& projection);

private:
    bool enabled;
    glm::vec3 position;
    glm::vec3 baseColor;
    float radius;
    float flickerTimer;
    float flickerIntensity;

    unsigned int glowVAO, glowVBO;
    int glowVertexCount;

    void buildGlowMesh();
};

#endif
