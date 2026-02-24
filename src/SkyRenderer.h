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

    // Update time of day (cycles over dayLengthSeconds)
    void update(float dt);

    // Render sky background (call before maze rendering, with depth write off)
    void render(const glm::mat4& view, const glm::mat4& projection);

    // Get current sun/moon direction for lighting
    glm::vec3 getSunDirection() const;

    // Get current sun color for lighting
    glm::vec3 getSunColor() const;

    // Get current fog color based on time of day
    glm::vec3 getFogColor() const;

    // Get current ambient light level (0-1)
    float getAmbientLevel() const;

    // Time of day (0.0 = midnight, 0.25 = sunrise, 0.5 = noon, 0.75 = sunset)
    float getTimeOfDay() const { return timeOfDay; }

private:
    unsigned int skyVAO, skyVBO;
    unsigned int sunVAO, sunVBO;
    int skyVertexCount;

    unsigned int skyShaderID;

    float timeOfDay;         // 0.0 to 1.0 (full day cycle)
    float dayLengthSeconds;  // real seconds for one full day

    void compileSkyShader();
    void buildSunMesh();

    glm::vec3 getSkyTopColor() const;
    glm::vec3 getSkyHorizonColor() const;
};

#endif
