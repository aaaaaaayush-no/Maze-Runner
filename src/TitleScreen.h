#ifndef TITLESCREEN_H
#define TITLESCREEN_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <vector>
#include <string>
#include "StarRating.h"

// ── Game screen states ─────────────────────────────────────────────────────
enum class GameScreen {
    TITLE_SCREEN,
    PLAYING,
    WIN_SCREEN
};

// ── Title screen menu ──────────────────────────────────────────────────────
class TitleScreen {
public:
    TitleScreen();
    ~TitleScreen();

    void init();
    void cleanup();

    // Returns true if "New Game" was selected (should start playing)
    bool update(float dt, bool upPressed, bool downPressed,
                bool leftPressed, bool rightPressed, bool enterPressed);

    void render(int screenWidth, int screenHeight, float time,
                const std::vector<int>& bestStars);

    Difficulty getSelectedDifficulty() const { return selectedDifficulty; }
    bool shouldQuit() const { return quitSelected; }

private:
    unsigned int vao, vbo;
    unsigned int shaderID;

    int menuIndex;           // 0 = New Game, 1 = Settings, 2 = Quit
    bool inSettings;         // sub-menu for difficulty
    Difficulty selectedDifficulty;
    bool quitSelected;

    // Debounce
    bool prevUp, prevDown, prevLeft, prevRight, prevEnter;

    // Animation
    float bobTimer;

    // Shader setup (uses HUD-style 2D rendering)
    void compileShader();

    // Drawing helpers
    void pushQuad(std::vector<float>& v, float x0, float y0, float x1, float y1,
                  float r, float g, float b);
    void renderBlockChar(std::vector<float>& verts, float x, float y,
                         float pixelSize, char c, float r, float g, float b);
    void renderBlockText(std::vector<float>& verts, const std::string& text,
                         float x, float y, float pixelSize,
                         float r, float g, float b);
    float getTextWidth(const std::string& text, float pixelSize) const;

    void drawBackground(std::vector<float>& verts, float time);
    void drawTitle(std::vector<float>& verts, float time);
    void drawMenu(std::vector<float>& verts, float time);
    void drawDifficultySelector(std::vector<float>& verts, float time,
                                const std::vector<int>& bestStars);

    void flush(const std::vector<float>& verts);
};

#endif
