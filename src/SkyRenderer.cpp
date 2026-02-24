#include "SkyRenderer.h"
#include <glm/gtc/matrix_transform.hpp>
#include <cmath>
#include <iostream>
#include <fstream>
#include <sstream>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// Inline sky shader sources (simple 2D colored quads, same as HUD format)
static const char* SKY_VERTEX_SRC = R"(
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;

out vec3 vColor;

uniform mat4 viewProj;

void main() {
    gl_Position = viewProj * vec4(aPos, 1.0);
    gl_Position.z = gl_Position.w; // push to far plane
    vColor = aColor;
}
)";

static const char* SKY_FRAGMENT_SRC = R"(
#version 330 core
in vec3 vColor;
out vec4 FragColor;

void main() {
    FragColor = vec4(vColor, 1.0);
}
)";

SkyRenderer::SkyRenderer()
    : skyVAO(0), skyVBO(0), sunVAO(0), sunVBO(0)
    , skyVertexCount(0), sunVertexCount(0)
    , skyShaderID(0)
    , timeOfDay(0.3f)  // start at morning
    , dayLengthSeconds(300.0f)  // 5 minutes for a full day cycle
{
}

SkyRenderer::~SkyRenderer() {
    cleanup();
}

void SkyRenderer::cleanup() {
    if (skyVAO) { glDeleteVertexArrays(1, &skyVAO); skyVAO = 0; }
    if (skyVBO) { glDeleteBuffers(1, &skyVBO); skyVBO = 0; }
    if (sunVAO) { glDeleteVertexArrays(1, &sunVAO); sunVAO = 0; }
    if (sunVBO) { glDeleteBuffers(1, &sunVBO); sunVBO = 0; }
    if (skyShaderID) { glDeleteProgram(skyShaderID); skyShaderID = 0; }
}

void SkyRenderer::compileSkyShader() {
    unsigned int vert = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vert, 1, &SKY_VERTEX_SRC, nullptr);
    glCompileShader(vert);
    int success;
    glGetShaderiv(vert, GL_COMPILE_STATUS, &success);
    if (!success) {
        char log[512];
        glGetShaderInfoLog(vert, 512, nullptr, log);
        std::cerr << "Sky vertex shader error: " << log << std::endl;
    }

    unsigned int frag = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(frag, 1, &SKY_FRAGMENT_SRC, nullptr);
    glCompileShader(frag);
    glGetShaderiv(frag, GL_COMPILE_STATUS, &success);
    if (!success) {
        char log[512];
        glGetShaderInfoLog(frag, 512, nullptr, log);
        std::cerr << "Sky fragment shader error: " << log << std::endl;
    }

    skyShaderID = glCreateProgram();
    glAttachShader(skyShaderID, vert);
    glAttachShader(skyShaderID, frag);
    glLinkProgram(skyShaderID);
    glGetProgramiv(skyShaderID, GL_LINK_STATUS, &success);
    if (!success) {
        char log[512];
        glGetProgramInfoLog(skyShaderID, 512, nullptr, log);
        std::cerr << "Sky shader link error: " << log << std::endl;
    }

    glDeleteShader(vert);
    glDeleteShader(frag);
}

void SkyRenderer::init() {
    compileSkyShader();
    buildSunMesh();
}

void SkyRenderer::update(float dt) {
    timeOfDay += dt / dayLengthSeconds;
    if (timeOfDay >= 1.0f) timeOfDay -= 1.0f;
}

glm::vec3 SkyRenderer::getSkyTopColor() const {
    // Time of day: 0.0=midnight, 0.25=sunrise, 0.5=noon, 0.75=sunset
    float t = timeOfDay;

    // Night: dark blue
    glm::vec3 nightTop(0.02f, 0.02f, 0.08f);
    // Dawn: soft purple-blue
    glm::vec3 dawnTop(0.3f, 0.35f, 0.6f);
    // Day: bright blue
    glm::vec3 dayTop(0.25f, 0.5f, 0.85f);
    // Sunset: deep orange-blue
    glm::vec3 sunsetTop(0.3f, 0.25f, 0.5f);

    if (t < 0.2f) {
        // Night
        return nightTop;
    } else if (t < 0.3f) {
        // Night to dawn
        float f = (t - 0.2f) / 0.1f;
        return glm::mix(nightTop, dawnTop, f);
    } else if (t < 0.4f) {
        // Dawn to day
        float f = (t - 0.3f) / 0.1f;
        return glm::mix(dawnTop, dayTop, f);
    } else if (t < 0.65f) {
        // Day
        return dayTop;
    } else if (t < 0.75f) {
        // Day to sunset
        float f = (t - 0.65f) / 0.1f;
        return glm::mix(dayTop, sunsetTop, f);
    } else if (t < 0.85f) {
        // Sunset to night
        float f = (t - 0.75f) / 0.1f;
        return glm::mix(sunsetTop, nightTop, f);
    } else {
        // Night
        return nightTop;
    }
}

glm::vec3 SkyRenderer::getSkyHorizonColor() const {
    float t = timeOfDay;

    glm::vec3 nightHorizon(0.03f, 0.03f, 0.06f);
    glm::vec3 dawnHorizon(0.8f, 0.5f, 0.3f);
    glm::vec3 dayHorizon(0.6f, 0.75f, 0.9f);
    glm::vec3 sunsetHorizon(0.9f, 0.4f, 0.15f);

    if (t < 0.2f) {
        return nightHorizon;
    } else if (t < 0.3f) {
        float f = (t - 0.2f) / 0.1f;
        return glm::mix(nightHorizon, dawnHorizon, f);
    } else if (t < 0.4f) {
        float f = (t - 0.3f) / 0.1f;
        return glm::mix(dawnHorizon, dayHorizon, f);
    } else if (t < 0.65f) {
        return dayHorizon;
    } else if (t < 0.75f) {
        float f = (t - 0.65f) / 0.1f;
        return glm::mix(dayHorizon, sunsetHorizon, f);
    } else if (t < 0.85f) {
        float f = (t - 0.75f) / 0.1f;
        return glm::mix(sunsetHorizon, nightHorizon, f);
    } else {
        return nightHorizon;
    }
}

glm::vec3 SkyRenderer::getSunDirection() const {
    // Sun arc: rises at t=0.25, peaks at t=0.5, sets at t=0.75
    // Moon arc: rises at t=0.75, peaks at t=0.0, sets at t=0.25
    float t = timeOfDay;

    // Sun angle (0 at horizon east, PI/2 at zenith, PI at horizon west)
    float sunAngle;
    bool isSun = (t >= 0.2f && t <= 0.8f);

    if (isSun) {
        // Map 0.2-0.8 to 0-PI
        sunAngle = (t - 0.2f) / 0.6f * (float)M_PI;
    } else {
        // Moon
        float mt = (t >= 0.8f) ? (t - 0.8f) : (t + 0.2f);
        sunAngle = mt / 0.4f * (float)M_PI;
    }

    float y = std::sin(sunAngle);
    float xz = std::cos(sunAngle);

    return glm::normalize(glm::vec3(xz * 0.7f, y, xz * 0.3f));
}

glm::vec3 SkyRenderer::getSunColor() const {
    float t = timeOfDay;

    if (t >= 0.25f && t <= 0.75f) {
        // Daytime: warm yellow
        // Redder near sunrise/sunset
        float midDist = std::abs(t - 0.5f) / 0.25f;
        float warmth = midDist * 0.3f;
        return glm::vec3(1.0f, 0.9f - warmth * 0.3f, 0.75f - warmth * 0.4f);
    } else {
        // Nighttime: pale blue-white moonlight
        return glm::vec3(0.4f, 0.45f, 0.6f);
    }
}

glm::vec3 SkyRenderer::getFogColor() const {
    glm::vec3 horizon = getSkyHorizonColor();
    // Darken slightly for fog
    return horizon * 0.5f + glm::vec3(0.01f);
}

float SkyRenderer::getAmbientLevel() const {
    float t = timeOfDay;
    // Higher during day, lower at night
    if (t >= 0.3f && t <= 0.7f) {
        return 0.4f;
    } else if (t < 0.2f || t > 0.8f) {
        return 0.15f;
    } else if (t < 0.3f) {
        float f = (t - 0.2f) / 0.1f;
        return glm::mix(0.15f, 0.4f, f);
    } else {
        float f = (t - 0.7f) / 0.1f;
        return glm::mix(0.4f, 0.15f, f);
    }
}

void SkyRenderer::buildSunMesh() {
    // Blocky sun/moon: a simple quad
    std::vector<float> verts;
    float s = 1.0f; // size, will be scaled in render

    // Two triangles for a quad
    // Vertex format: pos(3) + color(3)
    float c = 1.0f; // placeholder color, overridden per-frame
    verts.insert(verts.end(), {-s, -s, 0.0f, c, c, c});
    verts.insert(verts.end(), { s, -s, 0.0f, c, c, c});
    verts.insert(verts.end(), { s,  s, 0.0f, c, c, c});
    verts.insert(verts.end(), {-s, -s, 0.0f, c, c, c});
    verts.insert(verts.end(), { s,  s, 0.0f, c, c, c});
    verts.insert(verts.end(), {-s,  s, 0.0f, c, c, c});

    sunVertexCount = 6;

    glGenVertexArrays(1, &sunVAO);
    glGenBuffers(1, &sunVBO);
    glBindVertexArray(sunVAO);
    glBindBuffer(GL_ARRAY_BUFFER, sunVBO);
    glBufferData(GL_ARRAY_BUFFER, verts.size() * sizeof(float), verts.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glBindVertexArray(0);
}

void SkyRenderer::render(const glm::mat4& view, const glm::mat4& projection) {
    if (!skyShaderID) return;

    glUseProgram(skyShaderID);

    // Remove translation from view matrix for skybox effect
    glm::mat4 skyView = glm::mat4(glm::mat3(view));
    glm::mat4 viewProj = projection * skyView;

    int vpLoc = glGetUniformLocation(skyShaderID, "viewProj");
    glUniformMatrix4fv(vpLoc, 1, GL_FALSE, &viewProj[0][0]);

    // Build sky dome vertices dynamically based on time of day
    glm::vec3 topColor = getSkyTopColor();
    glm::vec3 horizonColor = getSkyHorizonColor();

    std::vector<float> skyVerts;

    // Create a hemisphere of triangles with gradient coloring
    int slices = 16;
    int stacks = 8;
    float radius = 100.0f;

    for (int i = 0; i < stacks; i++) {
        float phi0 = (float)i / stacks * ((float)M_PI / 2.0f);
        float phi1 = (float)(i + 1) / stacks * ((float)M_PI / 2.0f);

        // Color interpolation based on elevation
        float t0 = (float)i / stacks;
        float t1 = (float)(i + 1) / stacks;
        glm::vec3 c0 = glm::mix(horizonColor, topColor, t0);
        glm::vec3 c1 = glm::mix(horizonColor, topColor, t1);

        for (int j = 0; j < slices; j++) {
            float theta0 = 2.0f * (float)M_PI * j / slices;
            float theta1 = 2.0f * (float)M_PI * (j + 1) / slices;

            float x00 = radius * std::cos(phi0) * std::cos(theta0);
            float y00 = radius * std::sin(phi0);
            float z00 = radius * std::cos(phi0) * std::sin(theta0);

            float x10 = radius * std::cos(phi0) * std::cos(theta1);
            float y10 = radius * std::sin(phi0);
            float z10 = radius * std::cos(phi0) * std::sin(theta1);

            float x01 = radius * std::cos(phi1) * std::cos(theta0);
            float y01 = radius * std::sin(phi1);
            float z01 = radius * std::cos(phi1) * std::sin(theta0);

            float x11 = radius * std::cos(phi1) * std::cos(theta1);
            float y11 = radius * std::sin(phi1);
            float z11 = radius * std::cos(phi1) * std::sin(theta1);

            // Triangle 1
            skyVerts.insert(skyVerts.end(), {x00, y00, z00, c0.r, c0.g, c0.b});
            skyVerts.insert(skyVerts.end(), {x10, y10, z10, c0.r, c0.g, c0.b});
            skyVerts.insert(skyVerts.end(), {x11, y11, z11, c1.r, c1.g, c1.b});

            // Triangle 2
            skyVerts.insert(skyVerts.end(), {x00, y00, z00, c0.r, c0.g, c0.b});
            skyVerts.insert(skyVerts.end(), {x11, y11, z11, c1.r, c1.g, c1.b});
            skyVerts.insert(skyVerts.end(), {x01, y01, z01, c1.r, c1.g, c1.b});
        }
    }

    // Bottom hemisphere (below horizon - darker)
    for (int j = 0; j < slices; j++) {
        float theta0 = 2.0f * (float)M_PI * j / slices;
        float theta1 = 2.0f * (float)M_PI * (j + 1) / slices;

        float x0 = radius * std::cos(theta0);
        float z0 = radius * std::sin(theta0);
        float x1 = radius * std::cos(theta1);
        float z1 = radius * std::sin(theta1);

        glm::vec3 bc = horizonColor * 0.5f;

        skyVerts.insert(skyVerts.end(), {0.0f, -radius * 0.3f, 0.0f, bc.r, bc.g, bc.b});
        skyVerts.insert(skyVerts.end(), {x0, 0.0f, z0, horizonColor.r, horizonColor.g, horizonColor.b});
        skyVerts.insert(skyVerts.end(), {x1, 0.0f, z1, horizonColor.r, horizonColor.g, horizonColor.b});
    }

    skyVertexCount = (int)(skyVerts.size() / 6);

    if (!skyVAO) {
        glGenVertexArrays(1, &skyVAO);
        glGenBuffers(1, &skyVBO);
    }

    glBindVertexArray(skyVAO);
    glBindBuffer(GL_ARRAY_BUFFER, skyVBO);
    glBufferData(GL_ARRAY_BUFFER, skyVerts.size() * sizeof(float), skyVerts.data(), GL_DYNAMIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // Render sky with depth write disabled
    glDepthMask(GL_FALSE);
    glDisable(GL_CULL_FACE);
    glDrawArrays(GL_TRIANGLES, 0, skyVertexCount);

    // Render sun/moon
    {
        glm::vec3 sunDir = getSunDirection();
        glm::vec3 sunPos = sunDir * 80.0f;
        float sunSize = 5.0f;

        // Determine if it's sun or moon
        bool isSun = (timeOfDay >= 0.2f && timeOfDay <= 0.8f);
        glm::vec3 bodyColor = isSun ?
            glm::vec3(1.0f, 0.95f, 0.5f) :  // warm yellow sun
            glm::vec3(0.8f, 0.85f, 0.95f);   // pale blue-white moon

        // Billboard matrix: sun always faces camera
        glm::mat4 sunModel = glm::translate(glm::mat4(1.0f), sunPos);
        // Extract rotation from skyView and apply inverse
        glm::mat3 rot = glm::mat3(skyView);
        glm::mat3 invRot = glm::transpose(rot);
        sunModel = sunModel * glm::mat4(invRot);
        sunModel = glm::scale(sunModel, glm::vec3(sunSize));

        glm::mat4 sunVP = projection * skyView * sunModel;
        glUniformMatrix4fv(vpLoc, 1, GL_FALSE, &sunVP[0][0]);

        // Update sun mesh colors
        std::vector<float> sunVerts;
        float s = 1.0f;
        // Blocky shape: render as a square with stepped edges for pixelated look
        float bSize = 0.3f;
        // Center block (3x3 grid of small squares for blocky look)
        for (int bx = -1; bx <= 1; bx++) {
            for (int by = -1; by <= 1; by++) {
                float ox = bx * bSize * 2.0f;
                float oy = by * bSize * 2.0f;
                float cr = bodyColor.r;
                float cg = bodyColor.g;
                float cb = bodyColor.b;
                // Slightly vary corner blocks
                if (std::abs(bx) + std::abs(by) == 2) {
                    cr *= 0.85f; cg *= 0.85f; cb *= 0.85f;
                }

                sunVerts.insert(sunVerts.end(), {ox - bSize, oy - bSize, 0.0f, cr, cg, cb});
                sunVerts.insert(sunVerts.end(), {ox + bSize, oy - bSize, 0.0f, cr, cg, cb});
                sunVerts.insert(sunVerts.end(), {ox + bSize, oy + bSize, 0.0f, cr, cg, cb});
                sunVerts.insert(sunVerts.end(), {ox - bSize, oy - bSize, 0.0f, cr, cg, cb});
                sunVerts.insert(sunVerts.end(), {ox + bSize, oy + bSize, 0.0f, cr, cg, cb});
                sunVerts.insert(sunVerts.end(), {ox - bSize, oy + bSize, 0.0f, cr, cg, cb});
            }
        }

        // Extra edge blocks for cross/plus shape
        float ecr = bodyColor.r * 0.9f;
        float ecg = bodyColor.g * 0.9f;
        float ecb = bodyColor.b * 0.9f;
        for (int d = -1; d <= 1; d += 2) {
            // Top/bottom extensions
            float ox = 0.0f;
            float oy = d * bSize * 4.0f;
            sunVerts.insert(sunVerts.end(), {ox - bSize, oy - bSize, 0.0f, ecr, ecg, ecb});
            sunVerts.insert(sunVerts.end(), {ox + bSize, oy - bSize, 0.0f, ecr, ecg, ecb});
            sunVerts.insert(sunVerts.end(), {ox + bSize, oy + bSize, 0.0f, ecr, ecg, ecb});
            sunVerts.insert(sunVerts.end(), {ox - bSize, oy - bSize, 0.0f, ecr, ecg, ecb});
            sunVerts.insert(sunVerts.end(), {ox + bSize, oy + bSize, 0.0f, ecr, ecg, ecb});
            sunVerts.insert(sunVerts.end(), {ox - bSize, oy + bSize, 0.0f, ecr, ecg, ecb});

            // Left/right extensions
            ox = d * bSize * 4.0f;
            oy = 0.0f;
            sunVerts.insert(sunVerts.end(), {ox - bSize, oy - bSize, 0.0f, ecr, ecg, ecb});
            sunVerts.insert(sunVerts.end(), {ox + bSize, oy - bSize, 0.0f, ecr, ecg, ecb});
            sunVerts.insert(sunVerts.end(), {ox + bSize, oy + bSize, 0.0f, ecr, ecg, ecb});
            sunVerts.insert(sunVerts.end(), {ox - bSize, oy - bSize, 0.0f, ecr, ecg, ecb});
            sunVerts.insert(sunVerts.end(), {ox + bSize, oy + bSize, 0.0f, ecr, ecg, ecb});
            sunVerts.insert(sunVerts.end(), {ox - bSize, oy + bSize, 0.0f, ecr, ecg, ecb});
        }

        int sunVCount = (int)(sunVerts.size() / 6);

        glBindVertexArray(sunVAO);
        glBindBuffer(GL_ARRAY_BUFFER, sunVBO);
        glBufferData(GL_ARRAY_BUFFER, sunVerts.size() * sizeof(float), sunVerts.data(), GL_DYNAMIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);

        glDrawArrays(GL_TRIANGLES, 0, sunVCount);
    }

    glDepthMask(GL_TRUE);
    glEnable(GL_CULL_FACE);
    glBindVertexArray(0);
}
