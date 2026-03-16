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

    // Update: advances timeOfDay based on dt
    void update(float dt);

    // Render sky background (call before maze rendering, with depth write off)
    void render(const glm::mat4& view, const glm::mat4& projection);

    // Get current sun/moon direction for lighting
    glm::vec3 getSunDirection() const;

    // Get current sun/moon color for lighting
    glm::vec3 getSunColor() const;

    // Get current fog color based on time of day
    glm::vec3 getFogColor() const;

    // Get current ambient light level (0-1)
    float getAmbientLevel() const;

    // Time of day in [0,1): 0=midnight, 0.25=sunrise, 0.5=noon, 0.75=sunset
    float getTimeOfDay() const { return timeOfDay; }

private:
    unsigned int skyVAO, skyVBO;
    unsigned int sunVAO, sunVBO;
    unsigned int starVAO, starVBO;
    int skyVertexCount;
    int starVertexCount;

    unsigned int skyShaderID;

    // [0,1): 0=midnight, 0.25=sunrise, 0.5=noon, 0.75=sunset
    float timeOfDay;

    // Speed: 1.0 / secondsPerDay
    static constexpr float DAY_CYCLE_SPEED = 1.0f / 90.0f;

    void compileSkyShader();
    void buildSunMesh();
    void buildStarMesh();

    glm::vec3 getSkyTopColor() const;
    glm::vec3 getSkyHorizonColor() const;
};

#endif
