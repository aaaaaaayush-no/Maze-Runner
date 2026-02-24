#ifndef SKYRENDERER_H
#define SKYRENDERER_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <vector>

class Shader;

class SkyRenderer {
public:
    SkyRenderer();
    ~SkyRenderer();

    void init();
    void cleanup();

    // Update (no-op: permanent night)
    void update(float dt);

    // Render sky background (call before maze rendering, with depth write off)
    void render(const glm::mat4& view, const glm::mat4& projection);

    // Get current moon direction for lighting
    glm::vec3 getSunDirection() const;

    // Get current moon color for lighting
    glm::vec3 getSunColor() const;

    // Get current fog color based on night sky
    glm::vec3 getFogColor() const;

    // Get current ambient light level (0-1)
    float getAmbientLevel() const;

    // Time of day (permanently 0.0 = midnight)
    float getTimeOfDay() const { return timeOfDay; }

private:
    unsigned int skyVAO, skyVBO;
    unsigned int sunVAO, sunVBO;
    unsigned int starVAO, starVBO;
    int skyVertexCount;
    int starVertexCount;

    unsigned int skyShaderID;

    float timeOfDay;         // permanently 0.0 (midnight)

    void compileSkyShader();
    void buildSunMesh();
    void buildStarMesh();

    glm::vec3 getSkyTopColor() const;
    glm::vec3 getSkyHorizonColor() const;
};

#endif
