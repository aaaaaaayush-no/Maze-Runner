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
    , starVAO(0), starVBO(0)
    , skyVertexCount(0), starVertexCount(0)
    , skyShaderID(0)
    , timeOfDay(0.0f)  // permanent midnight
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
    if (starVAO) { glDeleteVertexArrays(1, &starVAO); starVAO = 0; }
    if (starVBO) { glDeleteBuffers(1, &starVBO); starVBO = 0; }
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
    buildStarMesh();
}

void SkyRenderer::update(float /*dt*/) {
    // Permanent night: no time advancement
}

glm::vec3 SkyRenderer::getSkyTopColor() const {
    // Permanent night sky: deep blue-black
    return glm::vec3(0.02f, 0.02f, 0.08f);
}

glm::vec3 SkyRenderer::getSkyHorizonColor() const {
    // Permanent night horizon: very dark blue
    return glm::vec3(0.03f, 0.03f, 0.06f);
}

glm::vec3 SkyRenderer::getSunDirection() const {
    // Permanent moon at high angle (near zenith)
    float moonAngle = 0.5f * (float)M_PI; // moon at peak
    float y = std::sin(moonAngle);
    float xz = std::cos(moonAngle);
    return glm::normalize(glm::vec3(xz * 0.7f, y, xz * 0.3f));
}

glm::vec3 SkyRenderer::getSunColor() const {
    // Cool blue-white moonlight
    return glm::vec3(0.55f, 0.6f, 0.8f);
}

glm::vec3 SkyRenderer::getFogColor() const {
    glm::vec3 horizon = getSkyHorizonColor();
    return horizon * 0.5f + glm::vec3(0.01f);
}

float SkyRenderer::getAmbientLevel() const {
    // Dim nighttime ambient: 0.15
    return 0.15f;
}

void SkyRenderer::buildSunMesh() {
    // Blocky sun/moon: a simple quad (placeholder, rebuilt dynamically in render)
    std::vector<float> verts;
    float s = 1.0f;

    // Two triangles for a quad
    // Vertex format: pos(3) + color(3)
    float c = 1.0f; // placeholder color, overridden per-frame
    verts.insert(verts.end(), {-s, -s, 0.0f, c, c, c});
    verts.insert(verts.end(), { s, -s, 0.0f, c, c, c});
    verts.insert(verts.end(), { s,  s, 0.0f, c, c, c});
    verts.insert(verts.end(), {-s, -s, 0.0f, c, c, c});
    verts.insert(verts.end(), { s,  s, 0.0f, c, c, c});
    verts.insert(verts.end(), {-s,  s, 0.0f, c, c, c});

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

void SkyRenderer::buildStarMesh() {
    // Generate random pixel-style stars scattered across the sky dome
    std::vector<float> verts;
    const int NUM_STARS = 200;
    const float radius = 95.0f;

    // Simple pseudo-random star placement using a seed
    unsigned int seed = 42;
    auto nextRand = [&seed]() -> float {
        seed = seed * 1103515245u + 12345u;
        return (float)((seed >> 16) & 0x7FFF) / 32767.0f;
    };

    for (int i = 0; i < NUM_STARS; i++) {
        // Random position on upper hemisphere
        float theta = nextRand() * 2.0f * (float)M_PI;
        float phi = nextRand() * (float)M_PI * 0.45f + 0.05f; // above horizon

        float x = radius * std::cos(phi) * std::cos(theta);
        float y = radius * std::sin(phi);
        float z = radius * std::cos(phi) * std::sin(theta);

        // Star size: small pixel dots
        float starSize = 0.15f + nextRand() * 0.25f;

        // Star brightness variation
        float brightness = 0.6f + nextRand() * 0.4f;
        float cr = brightness;
        float cg = brightness;
        float cb = brightness * (0.95f + nextRand() * 0.05f); // slightly blue tint

        // Billboard quad (two triangles) - always face origin roughly
        // Use a simple cross pattern for pixel look
        glm::vec3 pos(x, y, z);
        glm::vec3 toCenter = glm::normalize(-pos);
        glm::vec3 up(0.0f, 1.0f, 0.0f);
        glm::vec3 right = glm::normalize(glm::cross(toCenter, up));
        glm::vec3 realUp = glm::normalize(glm::cross(right, toCenter));

        glm::vec3 p0 = pos + (-right - realUp) * starSize;
        glm::vec3 p1 = pos + ( right - realUp) * starSize;
        glm::vec3 p2 = pos + ( right + realUp) * starSize;
        glm::vec3 p3 = pos + (-right + realUp) * starSize;

        verts.insert(verts.end(), {p0.x, p0.y, p0.z, cr, cg, cb});
        verts.insert(verts.end(), {p1.x, p1.y, p1.z, cr, cg, cb});
        verts.insert(verts.end(), {p2.x, p2.y, p2.z, cr, cg, cb});
        verts.insert(verts.end(), {p0.x, p0.y, p0.z, cr, cg, cb});
        verts.insert(verts.end(), {p2.x, p2.y, p2.z, cr, cg, cb});
        verts.insert(verts.end(), {p3.x, p3.y, p3.z, cr, cg, cb});
    }

    starVertexCount = (int)(verts.size() / 6);

    glGenVertexArrays(1, &starVAO);
    glGenBuffers(1, &starVBO);
    glBindVertexArray(starVAO);
    glBindBuffer(GL_ARRAY_BUFFER, starVBO);
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

    // Render moon (permanent night)
    {
        glm::vec3 sunDir = getSunDirection();
        glm::vec3 sunPos = sunDir * 80.0f;
        float sunSize = 5.0f;

        // Moon color: pale blue-white
        glm::vec3 bodyColor(0.8f, 0.85f, 0.95f);

        // Billboard matrix: moon always faces camera
        glm::mat4 sunModel = glm::translate(glm::mat4(1.0f), sunPos);
        // Extract rotation from skyView and apply inverse
        glm::mat3 rot = glm::mat3(skyView);
        glm::mat3 invRot = glm::transpose(rot);
        sunModel = sunModel * glm::mat4(invRot);
        sunModel = glm::scale(sunModel, glm::vec3(sunSize));

        glm::mat4 sunVP = projection * skyView * sunModel;
        glUniformMatrix4fv(vpLoc, 1, GL_FALSE, &sunVP[0][0]);

        // Build pixelated moon mesh
        std::vector<float> sunVerts;
        float bSize = 0.3f;
        // Center block (3x3 grid of small squares for blocky look)
        for (int bx = -1; bx <= 1; bx++) {
            for (int by = -1; by <= 1; by++) {
                float ox = bx * bSize * 2.0f;
                float oy = by * bSize * 2.0f;
                float cr = bodyColor.r;
                float cg = bodyColor.g;
                float cb = bodyColor.b;
                // Slightly vary corner blocks for pixel texture
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

        // Extra edge blocks for rounded pixel moon shape
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

        // Render moonlight glow halo
        {
            std::vector<float> glowVerts;
            int glowSegments = 16;
            float glowRadius = bSize * 8.0f;
            glm::vec3 glowCol = bodyColor * 0.3f; // soft glow

            for (int seg = 0; seg < glowSegments; seg++) {
                float a0 = 2.0f * (float)M_PI * seg / glowSegments;
                float a1 = 2.0f * (float)M_PI * (seg + 1) / glowSegments;

                // Center vertex (brighter)
                glowVerts.insert(glowVerts.end(), {0.0f, 0.0f, 0.0f,
                    bodyColor.r * 0.5f, bodyColor.g * 0.5f, bodyColor.b * 0.5f});
                // Outer vertices (faded)
                glowVerts.insert(glowVerts.end(), {glowRadius * std::cos(a0), glowRadius * std::sin(a0), 0.0f,
                    glowCol.r, glowCol.g, glowCol.b});
                glowVerts.insert(glowVerts.end(), {glowRadius * std::cos(a1), glowRadius * std::sin(a1), 0.0f,
                    glowCol.r, glowCol.g, glowCol.b});
            }

            int glowVCount = (int)(glowVerts.size() / 6);
            glBindBuffer(GL_ARRAY_BUFFER, sunVBO);
            glBufferData(GL_ARRAY_BUFFER, glowVerts.size() * sizeof(float), glowVerts.data(), GL_DYNAMIC_DRAW);
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
            glEnableVertexAttribArray(0);
            glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
            glEnableVertexAttribArray(1);

            glDrawArrays(GL_TRIANGLES, 0, glowVCount);
        }
    }

    // Render stars
    if (starVAO && starVertexCount > 0) {
        glUniformMatrix4fv(vpLoc, 1, GL_FALSE, &viewProj[0][0]);
        glBindVertexArray(starVAO);
        glDrawArrays(GL_TRIANGLES, 0, starVertexCount);
    }

    glDepthMask(GL_TRUE);
    glEnable(GL_CULL_FACE);
    glBindVertexArray(0);
}
