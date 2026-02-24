#include "Minimap.h"
#include "Maze.h"
#include "Collectible.h"
#include "Shader.h"
#include <cmath>

static const float CELL_SIZE = 2.0f;

Minimap::Minimap() : visible(true), vao(0), vbo(0) {}

Minimap::~Minimap() {
    if (vao) { glDeleteVertexArrays(1, &vao); glDeleteBuffers(1, &vbo); }
}

void Minimap::init() {
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
}

void Minimap::markExplored(int gx, int gy) {
    explored.insert({gx, gy});
}

void Minimap::clearExplored() {
    explored.clear();
}

void Minimap::pushQuad2D(std::vector<float>& v,
                         float x0, float y0, float x1, float y1,
                         float r, float g, float b) {
    // Two triangles forming a quad
    // Vertex format: x, y, r, g, b
    v.insert(v.end(), {x0,y0, r,g,b});
    v.insert(v.end(), {x1,y0, r,g,b});
    v.insert(v.end(), {x1,y1, r,g,b});

    v.insert(v.end(), {x0,y0, r,g,b});
    v.insert(v.end(), {x1,y1, r,g,b});
    v.insert(v.end(), {x0,y1, r,g,b});
}

void Minimap::pushTriangle2D(std::vector<float>& v,
                             float x0, float y0,
                             float x1, float y1,
                             float x2, float y2,
                             float r, float g, float b) {
    v.insert(v.end(), {x0,y0, r,g,b});
    v.insert(v.end(), {x1,y1, r,g,b});
    v.insert(v.end(), {x2,y2, r,g,b});
}

void Minimap::render(Shader& shader, const Maze& maze,
                     const glm::vec3& playerPos, float playerYaw,
                     const std::vector<CollectibleItem>& items,
                     const glm::vec3& exitPos,
                     int screenWidth, int screenHeight) {
    if (!visible) return;

    std::vector<float> verts;

    int mw = maze.getWidth();
    int mh = maze.getHeight();

    // Minimap occupies 1/10th of screen in the top-right corner
    float mapScreenSize = (float)std::min(screenWidth, screenHeight) * 0.18f;
    float mapPixelW = mapScreenSize;
    float mapPixelH = mapScreenSize;

    // NDC coordinates for top-right corner with padding
    float pad = 10.0f;
    float ndcRight = 1.0f - (pad / screenWidth * 2.0f);
    float ndcTop   = 1.0f - (pad / screenHeight * 2.0f);
    float ndcW = mapPixelW / screenWidth * 2.0f;
    float ndcH = mapPixelH / screenHeight * 2.0f;
    float ndcLeft   = ndcRight - ndcW;
    float ndcBottom = ndcTop - ndcH;

    // Background
    pushQuad2D(verts, ndcLeft, ndcBottom, ndcRight, ndcTop, 0.0f, 0.0f, 0.0f);

    // Cell size in NDC
    float cellW = ndcW / mw;
    float cellH = ndcH / mh;

    // Draw cells
    for (int y = 0; y < mh; y++) {
        for (int x = 0; x < mw; x++) {
            bool isExplored = explored.count({x, y}) > 0;
            if (!isExplored) continue;

            float cx = ndcLeft + x * cellW;
            float cy = ndcBottom + y * cellH;

            if (maze.isWall(x, y)) {
                pushQuad2D(verts, cx, cy, cx + cellW, cy + cellH,
                           0.4f, 0.35f, 0.3f);
            } else {
                pushQuad2D(verts, cx, cy, cx + cellW, cy + cellH,
                           0.15f, 0.15f, 0.2f);
            }
        }
    }

    // Draw uncollected items
    for (auto& item : items) {
        if (item.collected) continue;
        int gx = (int)std::floor(item.position.x / CELL_SIZE);
        int gy = (int)std::floor(item.position.z / CELL_SIZE);
        if (explored.count({gx, gy}) == 0) continue;

        float cx = ndcLeft + gx * cellW + cellW * 0.25f;
        float cy = ndcBottom + gy * cellH + cellH * 0.25f;
        pushQuad2D(verts, cx, cy, cx + cellW * 0.5f, cy + cellH * 0.5f,
                   1.0f, 0.84f, 0.0f); // gold
    }

    // Draw exit marker
    {
        int gx = (int)std::floor(exitPos.x / CELL_SIZE);
        int gy = (int)std::floor(exitPos.z / CELL_SIZE);
        if (explored.count({gx, gy}) > 0) {
            float cx = ndcLeft + gx * cellW + cellW * 0.1f;
            float cy = ndcBottom + gy * cellH + cellH * 0.1f;
            pushQuad2D(verts, cx, cy, cx + cellW * 0.8f, cy + cellH * 0.8f,
                       0.0f, 1.0f, 0.3f); // green
        }
    }

    // Draw player arrow
    {
        float pgx = playerPos.x / CELL_SIZE;
        float pgy = playerPos.z / CELL_SIZE;
        float pcx = ndcLeft + pgx * cellW;
        float pcy = ndcBottom + pgy * cellH;
        float arrowSize = cellW * 1.5f;
        float angle = glm::radians(playerYaw + 90.0f);
        float ax = cos(angle) * arrowSize;
        float ay = sin(angle) * arrowSize;
        float bx = cos(angle + 2.5f) * arrowSize * 0.5f;
        float by = sin(angle + 2.5f) * arrowSize * 0.5f;
        float cx = cos(angle - 2.5f) * arrowSize * 0.5f;
        float cy = sin(angle - 2.5f) * arrowSize * 0.5f;
        pushTriangle2D(verts, pcx + ax, pcy + ay,
                       pcx + bx, pcy + by,
                       pcx + cx, pcy + cy,
                       1.0f, 1.0f, 1.0f);
    }

    // Upload and render
    shader.use();

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, verts.size() * sizeof(float), verts.data(), GL_DYNAMIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glDisable(GL_DEPTH_TEST);
    glDrawArrays(GL_TRIANGLES, 0, (int)(verts.size() / 5));
    glEnable(GL_DEPTH_TEST);

    glBindVertexArray(0);
}
