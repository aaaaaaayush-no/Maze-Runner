#include "Minimap.h"
#include "Maze.h"
#include "Collectible.h"
#include "Shader.h"
#include <cmath>
#include <algorithm>

static const float CELL_SIZE = 2.0f;
static const int VIEWPORT_CELLS = 15; // scrolling viewport size for large mazes

Minimap::Minimap()
    : visible(true), legendVisible(false), vao(0), vbo(0),
      viewCamX(0.0f), viewCamY(0.0f) {}

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
    viewCamX = 0.0f;
    viewCamY = 0.0f;
}

void Minimap::pushQuad2D(std::vector<float>& v,
                         float x0, float y0, float x1, float y1,
                         float r, float g, float b) {
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
                     int screenWidth, int screenHeight,
                     int difficulty, float dt) {
    if (!visible) return;

    std::vector<float> verts;

    int mw = maze.getWidth();
    int mh = maze.getHeight();

    // Minimap occupies ~18% of screen in the top-right corner
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

    // ── Determine viewport ──────────────────────────────────────────────
    // For HARD (31x31) and NIGHTMARE (41x41): scrolling 15x15 viewport
    // For EASY (15x15) and MEDIUM (21x21): show full maze
    bool useViewport = (difficulty >= 2); // HARD=2, NIGHTMARE=3

    float playerGridX = playerPos.x / CELL_SIZE;
    float playerGridY = playerPos.z / CELL_SIZE;

    int viewMinX = 0, viewMinY = 0;
    int viewMaxX = mw, viewMaxY = mh;
    int viewW = mw, viewH = mh;

    if (useViewport) {
        viewW = VIEWPORT_CELLS;
        viewH = VIEWPORT_CELLS;

        // Lerp the viewport camera toward the player position
        float targetCamX = playerGridX;
        float targetCamY = playerGridY;
        float lerpSpeed = 8.0f;
        viewCamX += (targetCamX - viewCamX) * std::min(1.0f, lerpSpeed * dt);
        viewCamY += (targetCamY - viewCamY) * std::min(1.0f, lerpSpeed * dt);

        // Calculate viewport bounds centered on camera
        viewMinX = (int)std::floor(viewCamX) - viewW / 2;
        viewMinY = (int)std::floor(viewCamY) - viewH / 2;

        // Clamp to maze bounds
        if (viewMinX < 0) viewMinX = 0;
        if (viewMinY < 0) viewMinY = 0;
        if (viewMinX + viewW > mw) viewMinX = mw - viewW;
        if (viewMinY + viewH > mh) viewMinY = mh - viewH;

        viewMaxX = viewMinX + viewW;
        viewMaxY = viewMinY + viewH;
    }

    // Cell size in NDC based on visible cells
    float cellW = ndcW / viewW;
    float cellH = ndcH / viewH;

    // ── Draw cells (North-up: X world = right, Z world = down) ──────────
    // Maze Z increases downward in world. On minimap: -Z = up = North
    // So we draw y=0 at top and y=max at bottom (invert Y)
    for (int y = viewMinY; y < viewMaxY; y++) {
        for (int x = viewMinX; x < viewMaxX; x++) {
            bool isExplored = explored.count({x, y}) > 0;

            // Map cell to minimap NDC position
            // North-up: world Z=0 at top of minimap, Z=max at bottom
            float cx = ndcLeft + (x - viewMinX) * cellW;
            float cy = ndcTop - (y - viewMinY + 1) * cellH; // invert Y for north-up

            if (!isExplored) {
                // Fog of war: black for unvisited
                pushQuad2D(verts, cx, cy, cx + cellW, cy + cellH, 0.0f, 0.0f, 0.0f);
                continue;
            }

            // NIGHTMARE fog: cells fade back if far from player
            float alpha = 1.0f;
            if (difficulty == 3) { // NIGHTMARE
                float dx = (float)x - playerGridX;
                float dy = (float)y - playerGridY;
                float dist = std::sqrt(dx * dx + dy * dy);
                if (dist > 8.0f) {
                    alpha = 0.5f; // faded
                } else if (dist > 3.0f) {
                    alpha = 1.0f; // fully revealed
                }
            }

            if (maze.isWall(x, y)) {
                pushQuad2D(verts, cx, cy, cx + cellW, cy + cellH,
                           0.4f * alpha, 0.35f * alpha, 0.3f * alpha);
            } else {
                pushQuad2D(verts, cx, cy, cx + cellW, cy + cellH,
                           0.15f * alpha, 0.15f * alpha, 0.2f * alpha);
            }

            // NIGHTMARE: dark overlay for faded cells
            if (difficulty == 3 && alpha < 1.0f) {
                pushQuad2D(verts, cx, cy, cx + cellW, cy + cellH,
                           0.0f, 0.0f, 0.0f); // semi-dark overlay
            }
        }
    }

    // ── Draw uncollected items ──────────────────────────────────────────
    for (auto& item : items) {
        if (item.collected) continue;
        int gx = (int)std::floor(item.position.x / CELL_SIZE);
        int gy = (int)std::floor(item.position.z / CELL_SIZE);
        if (explored.count({gx, gy}) == 0) continue;
        if (gx < viewMinX || gx >= viewMaxX || gy < viewMinY || gy >= viewMaxY) continue;

        float cx = ndcLeft + (gx - viewMinX) * cellW + cellW * 0.25f;
        float cy = ndcTop - (gy - viewMinY + 1) * cellH + cellH * 0.25f;
        pushQuad2D(verts, cx, cy, cx + cellW * 0.5f, cy + cellH * 0.5f,
                   1.0f, 0.84f, 0.0f); // gold
    }

    // ── Draw exit marker (blue square) ──────────────────────────────────
    {
        int gx = (int)std::floor(exitPos.x / CELL_SIZE);
        int gy = (int)std::floor(exitPos.z / CELL_SIZE);
        if (explored.count({gx, gy}) > 0 &&
            gx >= viewMinX && gx < viewMaxX && gy >= viewMinY && gy < viewMaxY) {
            float cx = ndcLeft + (gx - viewMinX) * cellW + cellW * 0.1f;
            float cy = ndcTop - (gy - viewMinY + 1) * cellH + cellH * 0.1f;
            pushQuad2D(verts, cx, cy, cx + cellW * 0.8f, cy + cellH * 0.8f,
                       0.2f, 0.4f, 1.0f); // blue
        }
    }

    // ── Draw player dot (green) + directional arrow (white) ─────────────
    {
        float pgx = playerGridX;
        float pgy = playerGridY;
        // Player position on minimap (north-up: Y inverted)
        float pcx = ndcLeft + (pgx - viewMinX) * cellW;
        float pcy = ndcTop - (pgy - viewMinY) * cellH;

        // Green dot
        float dotR = cellW * 0.8f;
        pushQuad2D(verts, pcx - dotR, pcy - dotR, pcx + dotR, pcy + dotR,
                   0.0f, 1.0f, 0.0f); // bright green #00FF00

        // White directional arrow
        // Player yaw: -90 = North (facing -Z), 0 = East (+X), 90 = South (+Z)
        // On minimap: North = up, so we need to convert yaw to minimap angle
        // Yaw -90 → arrow pointing up (North)
        // The minimap has Y-up, so angle 0 = right, 90 = up
        // playerYaw in game: -90 = looking along -Z (North)
        // Minimap angle: yaw + 90 gives 0 for North, but we need up=90deg in NDC
        // So: minimap_angle = -(playerYaw + 90) + 90 = -playerYaw
        // Actually: let's think in terms of the direction vector
        // In world: front.x = cos(yaw), front.z = sin(yaw)  (yaw in radians)
        // On minimap: right = +X, up = -Z (north)
        // So minimap_dx = cos(yaw_rad), minimap_dy = -sin(yaw_rad)
        float yawRad = glm::radians(playerYaw);
        float dirX = cos(yawRad);
        float dirY = -sin(yawRad); // negate because minimap Y is inverted from Z

        float arrowLen = cellW * 2.0f;
        float arrowWidth = cellW * 0.8f;

        // Arrow tip
        float tipX = pcx + dirX * arrowLen;
        float tipY = pcy + dirY * arrowLen;

        // Arrow base (perpendicular)
        float perpX = -dirY * arrowWidth;
        float perpY = dirX * arrowWidth;

        pushTriangle2D(verts,
                       tipX, tipY,
                       pcx - dirX * arrowLen * 0.3f + perpX,
                       pcy - dirY * arrowLen * 0.3f + perpY,
                       pcx - dirX * arrowLen * 0.3f - perpX,
                       pcy - dirY * arrowLen * 0.3f - perpY,
                       1.0f, 1.0f, 1.0f); // white
    }

    // ── Compass Rose (top-left of minimap) ──────────────────────────────
    {
        float crCx = ndcLeft + ndcW * 0.08f;
        float crCy = ndcTop - ndcH * 0.08f;
        float crSize = ndcW * 0.04f;

        // N triangle (red, pointing up)
        pushTriangle2D(verts,
                       crCx, crCy + crSize * 2.5f,
                       crCx - crSize * 0.6f, crCy + crSize * 0.3f,
                       crCx + crSize * 0.6f, crCy + crSize * 0.3f,
                       1.0f, 0.0f, 0.0f); // red

        // S triangle (white, pointing down, smaller)
        pushTriangle2D(verts,
                       crCx, crCy - crSize * 1.8f,
                       crCx - crSize * 0.4f, crCy - crSize * 0.2f,
                       crCx + crSize * 0.4f, crCy - crSize * 0.2f,
                       1.0f, 1.0f, 1.0f);

        // E triangle (white, pointing right, smaller)
        pushTriangle2D(verts,
                       crCx + crSize * 1.8f, crCy,
                       crCx + crSize * 0.2f, crCy + crSize * 0.4f,
                       crCx + crSize * 0.2f, crCy - crSize * 0.4f,
                       1.0f, 1.0f, 1.0f);

        // W triangle (white, pointing left, smaller)
        pushTriangle2D(verts,
                       crCx - crSize * 1.8f, crCy,
                       crCx - crSize * 0.2f, crCy + crSize * 0.4f,
                       crCx - crSize * 0.2f, crCy - crSize * 0.4f,
                       1.0f, 1.0f, 1.0f);
    }

    // ── Cardinal direction labels on minimap border ─────────────────────
    // Rendered as small colored quads that approximate N/S/E/W letters
    {
        float lblSize = ndcW * 0.015f;
        float midX = (ndcLeft + ndcRight) * 0.5f;
        float midY = (ndcBottom + ndcTop) * 0.5f;

        // N (top center) - red block
        pushQuad2D(verts, midX - lblSize, ndcTop - lblSize * 0.5f,
                   midX + lblSize, ndcTop + lblSize * 1.5f,
                   1.0f, 0.0f, 0.0f);

        // S (bottom center) - white block
        pushQuad2D(verts, midX - lblSize, ndcBottom - lblSize * 1.5f,
                   midX + lblSize, ndcBottom + lblSize * 0.5f,
                   1.0f, 1.0f, 1.0f);

        // E (right center) - white block
        pushQuad2D(verts, ndcRight - lblSize * 0.5f, midY - lblSize,
                   ndcRight + lblSize * 1.5f, midY + lblSize,
                   1.0f, 1.0f, 1.0f);

        // W (left center) - white block
        pushQuad2D(verts, ndcLeft - lblSize * 1.5f, midY - lblSize,
                   ndcLeft + lblSize * 0.5f, midY + lblSize,
                   1.0f, 1.0f, 1.0f);
    }

    // ── Legend panel (toggled with L) ────────────────────────────────────
    if (legendVisible) {
        // Panel below minimap
        float legTop = ndcBottom - ndcH * 0.02f;
        float legH = ndcH * 0.35f;
        float legBottom = legTop - legH;
        float legLeft = ndcLeft;
        float legRight = ndcRight;

        // Semi-transparent black background (approximated with dark color)
        pushQuad2D(verts, legLeft, legBottom, legRight, legTop,
                   0.0f, 0.0f, 0.0f);

        float rowH = legH / 5.0f;
        float iconSize = rowH * 0.5f;
        float iconLeft = legLeft + ndcW * 0.03f;
        float textLeft = iconLeft + iconSize * 2.5f;

        // Row 1: Green dot = Player
        float ry = legTop - rowH * 0.5f;
        pushQuad2D(verts, iconLeft, ry - iconSize, iconLeft + iconSize * 2, ry + iconSize,
                   0.0f, 1.0f, 0.0f);

        // Row 2: Blue square = Exit Portal
        ry -= rowH;
        pushQuad2D(verts, iconLeft, ry - iconSize, iconLeft + iconSize * 2, ry + iconSize,
                   0.2f, 0.4f, 1.0f);

        // Row 3: Gray = Wall
        ry -= rowH;
        pushQuad2D(verts, iconLeft, ry - iconSize, iconLeft + iconSize * 2, ry + iconSize,
                   0.4f, 0.35f, 0.3f);

        // Row 4: Light = Corridor
        ry -= rowH;
        pushQuad2D(verts, iconLeft, ry - iconSize, iconLeft + iconSize * 2, ry + iconSize,
                   0.15f, 0.15f, 0.2f);

        // Row 5: Black = Unexplored
        ry -= rowH;
        pushQuad2D(verts, iconLeft, ry - iconSize, iconLeft + iconSize * 2, ry + iconSize,
                   0.05f, 0.05f, 0.05f);
    }

    // ── Upload and render ───────────────────────────────────────────────
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
