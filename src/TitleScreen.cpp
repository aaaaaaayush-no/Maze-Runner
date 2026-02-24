#include "TitleScreen.h"
#include <cmath>
#include <cstring>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// ── 5x7 pixel font for blocky Minecraft-style characters ──────────────────
// Each character is a 5-wide x 7-tall bitmap stored as 7 bytes (1 bit per pixel).
static const unsigned char FONT_5x7[][7] = {
    // A
    {0b01110, 0b10001, 0b10001, 0b11111, 0b10001, 0b10001, 0b10001},
    // B
    {0b11110, 0b10001, 0b10001, 0b11110, 0b10001, 0b10001, 0b11110},
    // C
    {0b01110, 0b10001, 0b10000, 0b10000, 0b10000, 0b10001, 0b01110},
    // D
    {0b11100, 0b10010, 0b10001, 0b10001, 0b10001, 0b10010, 0b11100},
    // E
    {0b11111, 0b10000, 0b10000, 0b11110, 0b10000, 0b10000, 0b11111},
    // F
    {0b11111, 0b10000, 0b10000, 0b11110, 0b10000, 0b10000, 0b10000},
    // G
    {0b01110, 0b10001, 0b10000, 0b10111, 0b10001, 0b10001, 0b01110},
    // H
    {0b10001, 0b10001, 0b10001, 0b11111, 0b10001, 0b10001, 0b10001},
    // I
    {0b01110, 0b00100, 0b00100, 0b00100, 0b00100, 0b00100, 0b01110},
    // J
    {0b00111, 0b00010, 0b00010, 0b00010, 0b00010, 0b10010, 0b01100},
    // K
    {0b10001, 0b10010, 0b10100, 0b11000, 0b10100, 0b10010, 0b10001},
    // L
    {0b10000, 0b10000, 0b10000, 0b10000, 0b10000, 0b10000, 0b11111},
    // M
    {0b10001, 0b11011, 0b10101, 0b10101, 0b10001, 0b10001, 0b10001},
    // N
    {0b10001, 0b11001, 0b10101, 0b10011, 0b10001, 0b10001, 0b10001},
    // O
    {0b01110, 0b10001, 0b10001, 0b10001, 0b10001, 0b10001, 0b01110},
    // P
    {0b11110, 0b10001, 0b10001, 0b11110, 0b10000, 0b10000, 0b10000},
    // Q
    {0b01110, 0b10001, 0b10001, 0b10001, 0b10101, 0b10010, 0b01101},
    // R
    {0b11110, 0b10001, 0b10001, 0b11110, 0b10100, 0b10010, 0b10001},
    // S
    {0b01111, 0b10000, 0b10000, 0b01110, 0b00001, 0b00001, 0b11110},
    // T
    {0b11111, 0b00100, 0b00100, 0b00100, 0b00100, 0b00100, 0b00100},
    // U
    {0b10001, 0b10001, 0b10001, 0b10001, 0b10001, 0b10001, 0b01110},
    // V
    {0b10001, 0b10001, 0b10001, 0b10001, 0b10001, 0b01010, 0b00100},
    // W
    {0b10001, 0b10001, 0b10001, 0b10101, 0b10101, 0b11011, 0b10001},
    // X
    {0b10001, 0b10001, 0b01010, 0b00100, 0b01010, 0b10001, 0b10001},
    // Y
    {0b10001, 0b10001, 0b01010, 0b00100, 0b00100, 0b00100, 0b00100},
    // Z
    {0b11111, 0b00001, 0b00010, 0b00100, 0b01000, 0b10000, 0b11111},
};

// Digits 0-9
static const unsigned char FONT_DIGITS[][7] = {
    // 0
    {0b01110, 0b10001, 0b10011, 0b10101, 0b11001, 0b10001, 0b01110},
    // 1
    {0b00100, 0b01100, 0b00100, 0b00100, 0b00100, 0b00100, 0b01110},
    // 2
    {0b01110, 0b10001, 0b00001, 0b00110, 0b01000, 0b10000, 0b11111},
    // 3
    {0b01110, 0b10001, 0b00001, 0b00110, 0b00001, 0b10001, 0b01110},
    // 4
    {0b00010, 0b00110, 0b01010, 0b10010, 0b11111, 0b00010, 0b00010},
    // 5
    {0b11111, 0b10000, 0b11110, 0b00001, 0b00001, 0b10001, 0b01110},
    // 6
    {0b01110, 0b10000, 0b10000, 0b11110, 0b10001, 0b10001, 0b01110},
    // 7
    {0b11111, 0b00001, 0b00010, 0b00100, 0b01000, 0b01000, 0b01000},
    // 8
    {0b01110, 0b10001, 0b10001, 0b01110, 0b10001, 0b10001, 0b01110},
    // 9
    {0b01110, 0b10001, 0b10001, 0b01111, 0b00001, 0b00001, 0b01110},
};

TitleScreen::TitleScreen()
    : vao(0), vbo(0), shaderID(0)
    , menuIndex(0), inSettings(false)
    , selectedDifficulty(Difficulty::MEDIUM)
    , quitSelected(false)
    , prevUp(false), prevDown(false)
    , prevLeft(false), prevRight(false), prevEnter(false)
    , bobTimer(0.0f)
{
}

TitleScreen::~TitleScreen() {
    cleanup();
}

void TitleScreen::init() {
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
}

void TitleScreen::cleanup() {
    if (vao) { glDeleteVertexArrays(1, &vao); vao = 0; }
    if (vbo) { glDeleteBuffers(1, &vbo); vbo = 0; }
}

bool TitleScreen::update(float dt, bool upPressed, bool downPressed,
                         bool leftPressed, bool rightPressed, bool enterPressed) {
    bobTimer += dt;

    // Edge detection for navigation
    bool upEdge = upPressed && !prevUp;
    bool downEdge = downPressed && !prevDown;
    bool leftEdge = leftPressed && !prevLeft;
    bool rightEdge = rightPressed && !prevRight;
    bool enterEdge = enterPressed && !prevEnter;

    prevUp = upPressed;
    prevDown = downPressed;
    prevLeft = leftPressed;
    prevRight = rightPressed;
    prevEnter = enterPressed;

    if (inSettings) {
        // Difficulty selection with left/right
        int d = (int)selectedDifficulty;
        if (leftEdge) d = (d + 3) % 4;  // wrap left
        if (rightEdge) d = (d + 1) % 4;  // wrap right
        selectedDifficulty = (Difficulty)d;

        if (enterEdge || upEdge || downEdge) {
            inSettings = false;  // back to main menu
        }
        return false;
    }

    // Main menu navigation
    if (upEdge) menuIndex = (menuIndex + 2) % 3;   // wrap up
    if (downEdge) menuIndex = (menuIndex + 1) % 3;  // wrap down

    if (enterEdge) {
        switch (menuIndex) {
            case 0: return true;  // New Game
            case 1: inSettings = true; break;  // Settings
            case 2: quitSelected = true; break; // Quit
        }
    }

    return false;
}

void TitleScreen::pushQuad(std::vector<float>& v, float x0, float y0,
                           float x1, float y1, float r, float g, float b) {
    v.insert(v.end(), {x0, y0, r, g, b});
    v.insert(v.end(), {x1, y0, r, g, b});
    v.insert(v.end(), {x1, y1, r, g, b});
    v.insert(v.end(), {x0, y0, r, g, b});
    v.insert(v.end(), {x1, y1, r, g, b});
    v.insert(v.end(), {x0, y1, r, g, b});
}

void TitleScreen::renderBlockChar(std::vector<float>& verts, float x, float y,
                                  float pixelSize, char c, float r, float g, float b) {
    const unsigned char* bitmap = nullptr;

    if (c >= 'A' && c <= 'Z') {
        bitmap = FONT_5x7[c - 'A'];
    } else if (c >= 'a' && c <= 'z') {
        bitmap = FONT_5x7[c - 'a'];
    } else if (c >= '0' && c <= '9') {
        bitmap = FONT_DIGITS[c - '0'];
    } else if (c == ' ') {
        return; // space - no pixels
    } else if (c == '/') {
        // Simple diagonal
        for (int row = 0; row < 7; row++) {
            int col = 4 - (row * 5 / 7);
            float px = x + col * pixelSize;
            float py = y + (6 - row) * pixelSize;
            pushQuad(verts, px, py, px + pixelSize, py + pixelSize, r, g, b);
        }
        return;
    } else if (c == ':') {
        float px = x + 2 * pixelSize;
        pushQuad(verts, px, y + 2 * pixelSize, px + pixelSize, y + 3 * pixelSize, r, g, b);
        pushQuad(verts, px, y + 4 * pixelSize, px + pixelSize, y + 5 * pixelSize, r, g, b);
        return;
    } else if (c == '-') {
        float px = x + pixelSize;
        float py = y + 3 * pixelSize;
        pushQuad(verts, px, py, px + 3 * pixelSize, py + pixelSize, r, g, b);
        return;
    } else if (c == '>') {
        // Arrow right
        for (int row = 0; row < 7; row++) {
            int col;
            if (row <= 3) col = row;
            else col = 6 - row;
            float px = x + col * pixelSize;
            float py = y + (6 - row) * pixelSize;
            pushQuad(verts, px, py, px + pixelSize, py + pixelSize, r, g, b);
        }
        return;
    } else if (c == '<') {
        for (int row = 0; row < 7; row++) {
            int col;
            if (row <= 3) col = 3 - row;
            else col = row - 3;
            float px = x + col * pixelSize;
            float py = y + (6 - row) * pixelSize;
            pushQuad(verts, px, py, px + pixelSize, py + pixelSize, r, g, b);
        }
        return;
    } else {
        return; // unknown char
    }

    if (!bitmap) return;

    // Render the 5x7 bitmap
    for (int row = 0; row < 7; row++) {
        unsigned char rowBits = bitmap[row];
        for (int col = 0; col < 5; col++) {
            if (rowBits & (1 << (4 - col))) {
                float px = x + col * pixelSize;
                float py = y + (6 - row) * pixelSize; // flip Y (top row = highest y)
                pushQuad(verts, px, py, px + pixelSize, py + pixelSize, r, g, b);
            }
        }
    }
}

void TitleScreen::renderBlockText(std::vector<float>& verts, const std::string& text,
                                  float x, float y, float pixelSize,
                                  float r, float g, float b) {
    float cursorX = x;
    for (size_t i = 0; i < text.size(); i++) {
        renderBlockChar(verts, cursorX, y, pixelSize, text[i], r, g, b);
        cursorX += 6.0f * pixelSize; // 5 pixels + 1 spacing
    }
}

float TitleScreen::getTextWidth(const std::string& text, float pixelSize) const {
    if (text.empty()) return 0.0f;
    return text.size() * 6.0f * pixelSize - pixelSize; // last char has no spacing
}

void TitleScreen::drawBackground(std::vector<float>& verts, float time) {
    // Minecraft-style dirt/stone pattern background
    // Grid of slightly different colored blocks
    float blockSize = 0.08f;
    for (float y = -1.0f; y < 1.0f; y += blockSize) {
        for (float x = -1.0f; x < 1.0f; x += blockSize) {
            // Pseudo-random color variation based on position
            int ix = (int)((x + 1.0f) / blockSize);
            int iy = (int)((y + 1.0f) / blockSize);
            unsigned int hash = (unsigned int)(ix * 7919 + iy * 104729);
            hash = ((hash >> 16) ^ hash) * 0x45d9f3b;
            hash = ((hash >> 16) ^ hash) * 0x45d9f3b;
            hash = (hash >> 16) ^ hash;

            float variation = (float)(hash % 100) / 100.0f;

            // Dark dirt/stone colors
            float r = 0.18f + variation * 0.08f;
            float g = 0.14f + variation * 0.06f;
            float b = 0.10f + variation * 0.04f;

            // Inner block (slightly inset for grid look)
            float inset = blockSize * 0.05f;
            pushQuad(verts, x + inset, y + inset,
                     x + blockSize - inset, y + blockSize - inset,
                     r, g, b);

            // Edge lines (darker)
            float er = r * 0.6f, eg = g * 0.6f, eb = b * 0.6f;
            float edge = blockSize * 0.05f;
            // Bottom edge
            pushQuad(verts, x, y, x + blockSize, y + edge, er, eg, eb);
            // Left edge
            pushQuad(verts, x, y, x + edge, y + blockSize, er, eg, eb);
        }
    }

    // Subtle gradient overlay (darker at top and bottom)
    pushQuad(verts, -1.0f, 0.7f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f);
    pushQuad(verts, -1.0f, -1.0f, 1.0f, -0.7f, 0.0f, 0.0f, 0.0f);

    // Semi-transparent vignette overlay (darker edges)
    float vig = 0.03f;
    for (float vy = -1.0f; vy < 1.0f; vy += 0.1f) {
        float distFromCenter = std::abs(vy);
        float alpha = distFromCenter * 0.15f;
        pushQuad(verts, -1.0f, vy, 1.0f, vy + 0.1f,
                 alpha * 0.02f, alpha * 0.01f, alpha * 0.03f);
    }
    (void)vig;
    (void)time;
}

void TitleScreen::drawTitle(std::vector<float>& verts, float time) {
    // "MAZE RUNNER" in large blocky pixels
    float titlePixel = 0.016f;
    std::string title = "MAZE RUNNER";
    float titleWidth = getTextWidth(title, titlePixel);

    // Gentle bob animation
    float bob = std::sin(time * 1.5f) * 0.015f;
    float titleX = -titleWidth / 2.0f;
    float titleY = 0.45f + bob;

    // Shadow/outline (render slightly offset in dark color)
    float shadowOff = titlePixel * 0.5f;
    renderBlockText(verts, title, titleX + shadowOff, titleY - shadowOff,
                    titlePixel, 0.1f, 0.08f, 0.05f);

    // Main title in bright yellow (Minecraft title style)
    renderBlockText(verts, title, titleX, titleY,
                    titlePixel, 1.0f, 0.84f, 0.0f);

    // Subtitle
    std::string subtitle = "A MINECRAFT ADVENTURE";
    float subPixel = 0.006f;
    float subWidth = getTextWidth(subtitle, subPixel);
    renderBlockText(verts, subtitle, -subWidth / 2.0f, 0.35f,
                    subPixel, 0.7f, 0.7f, 0.7f);
}

void TitleScreen::drawMenu(std::vector<float>& verts, float time) {
    const char* items[] = {"NEW GAME", "SETTINGS", "QUIT"};
    float menuPixel = 0.008f;
    float startY = 0.05f;
    float spacing = 0.12f;

    for (int i = 0; i < 3; i++) {
        float itemBob = 0.0f;
        float r = 0.8f, g = 0.8f, b = 0.8f;

        if (i == menuIndex && !inSettings) {
            // Selected item: yellow with bobbing
            r = 1.0f; g = 1.0f; b = 0.3f;
            itemBob = std::sin(time * 4.0f) * 0.005f;

            // Draw selection arrow
            std::string arrow = ">";
            float arrowX = -0.35f;
            float arrowY = startY - i * spacing + itemBob;
            renderBlockText(verts, arrow, arrowX, arrowY, menuPixel, r, g, b);
        }

        std::string text = items[i];
        float textWidth = getTextWidth(text, menuPixel);
        float textX = -textWidth / 2.0f;
        float textY = startY - i * spacing + itemBob;

        // Shadow
        renderBlockText(verts, text, textX + menuPixel * 0.3f,
                        textY - menuPixel * 0.3f, menuPixel,
                        0.1f, 0.1f, 0.1f);
        // Main text
        renderBlockText(verts, text, textX, textY, menuPixel, r, g, b);
    }
}

void TitleScreen::drawDifficultySelector(std::vector<float>& verts, float time,
                                         const std::vector<int>& bestStars) {
    if (!inSettings) return;

    // Dark overlay
    pushQuad(verts, -0.6f, -0.35f, 0.6f, 0.25f, 0.05f, 0.05f, 0.08f);
    // Border
    float bw = 0.005f;
    pushQuad(verts, -0.6f, -0.35f, 0.6f, -0.35f + bw, 0.5f, 0.4f, 0.2f);
    pushQuad(verts, -0.6f, 0.25f - bw, 0.6f, 0.25f, 0.5f, 0.4f, 0.2f);
    pushQuad(verts, -0.6f, -0.35f, -0.6f + bw, 0.25f, 0.5f, 0.4f, 0.2f);
    pushQuad(verts, 0.6f - bw, -0.35f, 0.6f, 0.25f, 0.5f, 0.4f, 0.2f);

    // Title
    float pixel = 0.007f;
    std::string selectTitle = "SELECT DIFFICULTY";
    float titleW = getTextWidth(selectTitle, pixel);
    renderBlockText(verts, selectTitle, -titleW / 2.0f, 0.15f, pixel,
                    1.0f, 0.84f, 0.0f);

    // Difficulty options
    const char* names[] = {"EASY", "MEDIUM", "HARD", "NIGHTMARE"};
    float colors[][3] = {
        {0.3f, 0.9f, 0.3f},  // green
        {0.9f, 0.9f, 0.3f},  // yellow
        {0.9f, 0.5f, 0.2f},  // orange
        {0.9f, 0.2f, 0.2f},  // red
    };

    int sel = (int)selectedDifficulty;
    float namePixel = 0.009f;
    std::string name = names[sel];
    float nameW = getTextWidth(name, namePixel);

    float bob = std::sin(time * 3.0f) * 0.005f;

    // Arrows
    renderBlockText(verts, "<", -0.3f, 0.0f + bob, namePixel, 0.8f, 0.8f, 0.8f);
    renderBlockText(verts, ">",  0.25f, 0.0f + bob, namePixel, 0.8f, 0.8f, 0.8f);

    // Difficulty name
    renderBlockText(verts, name, -nameW / 2.0f, 0.0f + bob, namePixel,
                    colors[sel][0], colors[sel][1], colors[sel][2]);

    // Show best stars for this difficulty
    if (sel < (int)bestStars.size()) {
        int stars = bestStars[sel];
        float starPixel = 0.005f;
        std::string starStr;
        for (int s = 0; s < 3; s++) {
            starStr += (s < stars) ? "O" : "-";
            if (s < 2) starStr += " ";
        }
        float starW = getTextWidth(starStr, starPixel);
        renderBlockText(verts, starStr, -starW / 2.0f, -0.1f, starPixel,
                        1.0f, 0.84f, 0.0f);
    }

    // Instructions
    float instrPixel = 0.004f;
    std::string instr = "PRESS ENTER TO CONFIRM";
    float instrW = getTextWidth(instr, instrPixel);
    renderBlockText(verts, instr, -instrW / 2.0f, -0.25f, instrPixel,
                    0.5f, 0.5f, 0.5f);
}

void TitleScreen::flush(const std::vector<float>& verts) {
    if (verts.empty()) return;

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

void TitleScreen::render(int /*screenWidth*/, int /*screenHeight*/, float time,
                         const std::vector<int>& bestStars) {
    std::vector<float> verts;

    drawBackground(verts, time);
    drawTitle(verts, time);
    drawMenu(verts, time);
    drawDifficultySelector(verts, time, bestStars);

    // Instructions at bottom
    float instrPixel = 0.004f;
    std::string instr = "USE WASD OR ARROWS TO NAVIGATE";
    float instrW = getTextWidth(instr, instrPixel);
    renderBlockText(verts, instr, -instrW / 2.0f, -0.85f, instrPixel,
                    0.4f, 0.4f, 0.5f);

    flush(verts);
}
