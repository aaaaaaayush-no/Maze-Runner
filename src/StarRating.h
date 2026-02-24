#ifndef STARRATING_H
#define STARRATING_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <vector>
#include <cmath>

// ── Difficulty levels ──────────────────────────────────────────────────────
enum class Difficulty {
    EASY = 0,
    MEDIUM,
    HARD,
    NIGHTMARE
};

struct DifficultyConfig {
    int mazeWidth;
    int mazeHeight;
    int numItems;
    float threeStar;  // seconds
    float twoStar;    // seconds
    const char* name;
};

inline DifficultyConfig getDifficultyConfig(Difficulty d) {
    switch (d) {
        case Difficulty::EASY:      return {15, 15,  5,  60.0f, 120.0f, "EASY"};
        case Difficulty::MEDIUM:    return {21, 21,  7,  90.0f, 180.0f, "MEDIUM"};
        case Difficulty::HARD:      return {31, 31, 10, 150.0f, 300.0f, "HARD"};
        case Difficulty::NIGHTMARE: return {41, 41, 14, 240.0f, 480.0f, "NIGHTMARE"};
    }
    return {21, 21, 7, 90.0f, 180.0f, "MEDIUM"};
}

// ── Star calculation ───────────────────────────────────────────────────────
struct StarResult {
    int stars;        // 1-3
    bool perfectRun;  // all collectables + under 3-star time
};

inline StarResult calculateStars(Difficulty diff, float timeSec,
                                 bool allCollected) {
    auto cfg = getDifficultyConfig(diff);
    StarResult result;
    result.perfectRun = false;

    if (timeSec < cfg.threeStar)
        result.stars = 3;
    else if (timeSec < cfg.twoStar)
        result.stars = 2;
    else
        result.stars = 1;

    // Cap at 2 stars if not all collectables collected
    if (!allCollected && result.stars > 2)
        result.stars = 2;

    // Perfect run: all collected AND 3 stars
    if (allCollected && result.stars == 3)
        result.perfectRun = true;

    return result;
}

// ── Star shape vertex generation (for HUD shader) ─────────────────────────
// Generates a 10-pointed star polygon (5 outer, 5 inner) as triangles
// in 2D NDC space. Vertex format: x, y, r, g, b
inline void generateStarVerts(std::vector<float>& verts,
                              float cx, float cy,
                              float outerR, float innerR,
                              float r, float g, float b,
                              float scale = 1.0f) {
    int points = 5;
    float oR = outerR * scale;
    float iR = innerR * scale;

    // Triangle fan: center + ring of alternating outer/inner points
    // Generate as individual triangles for GL_TRIANGLES
    float prevX = cx + cos(-M_PI / 2.0f) * oR;
    float prevY = cy + sin(-M_PI / 2.0f) * oR;

    for (int i = 1; i <= points * 2; i++) {
        float angle = i * (float)M_PI / points - (float)M_PI / 2.0f;
        float radius = (i % 2 == 0) ? oR : iR;
        float px = cx + cos(angle) * radius;
        float py = cy + sin(angle) * radius;

        verts.insert(verts.end(), {cx, cy, r, g, b});
        verts.insert(verts.end(), {prevX, prevY, r, g, b});
        verts.insert(verts.end(), {px, py, r, g, b});

        prevX = px;
        prevY = py;
    }
}

// Generate star outline (ring of line segments as thin quads)
inline void generateStarOutline(std::vector<float>& verts,
                                float cx, float cy,
                                float outerR, float innerR,
                                float r, float g, float b,
                                float scale = 1.0f,
                                float thickness = 0.002f) {
    int points = 5;
    float oR = outerR * scale;
    float iR = innerR * scale;

    for (int i = 0; i <= points * 2; i++) {
        float angle0 = i * (float)M_PI / points - (float)M_PI / 2.0f;
        float angle1 = (i + 1) * (float)M_PI / points - (float)M_PI / 2.0f;
        float r0 = (i % 2 == 0) ? oR : iR;
        float r1 = ((i + 1) % 2 == 0) ? oR : iR;

        float x0 = cx + cos(angle0) * r0;
        float y0 = cy + sin(angle0) * r0;
        float x1 = cx + cos(angle1) * r1;
        float y1 = cy + sin(angle1) * r1;

        // Direction perpendicular to the edge
        float dx = x1 - x0;
        float dy = y1 - y0;
        float len = sqrt(dx * dx + dy * dy);
        if (len < 0.0001f) continue;
        float nx = -dy / len * thickness;
        float ny = dx / len * thickness;

        // Quad as two triangles
        verts.insert(verts.end(), {x0 + nx, y0 + ny, r, g, b});
        verts.insert(verts.end(), {x1 + nx, y1 + ny, r, g, b});
        verts.insert(verts.end(), {x1 - nx, y1 - ny, r, g, b});

        verts.insert(verts.end(), {x0 + nx, y0 + ny, r, g, b});
        verts.insert(verts.end(), {x1 - nx, y1 - ny, r, g, b});
        verts.insert(verts.end(), {x0 - nx, y0 - ny, r, g, b});
    }
}

// ── Particle for star burst effect ────────────────────────────────────────
struct StarParticle {
    float x, y;      // position (NDC)
    float vx, vy;    // velocity
    float life;       // remaining life (seconds)
    float maxLife;
};

#endif
