#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Shader.h"
#include "Maze.h"
#include "Player.h"
#include "Renderer.h"
#include "Minimap.h"
#include "Collectible.h"
#include "StarRating.h"
#include "Highscore.h"
#include "TitleScreen.h"
#include "HandRenderer.h"
#include "SkyRenderer.h"
#include "TorchLight.h"

#include <iostream>
#include <string>
#include <cmath>
#include <cstdio>
#include <vector>
#include <algorithm>

// ── Configuration ──────────────────────────────────────────────────────────
static const float CELL_SIZE = 2.0f;
static const float FIXED_DT  = 1.0f / 60.0f;
static const std::string HIGHSCORE_FILE = "highscores.txt";

// ── Globals ────────────────────────────────────────────────────────────────
static int screenWidth  = 1280;
static int screenHeight = 720;

static float lastMouseX = screenWidth / 2.0f;
static float lastMouseY = screenHeight / 2.0f;
static bool  firstMouse = true;

static bool keyW = false, keyA = false, keyS = false, keyD = false;
static bool keySpace = false;
static bool requestRestart = false;
static bool requestWireToggle = false;
static bool requestMinimapToggle = false;
static bool requestLegendToggle = false;
static bool requestTorchToggle = false;

static Difficulty currentDifficulty = Difficulty::MEDIUM;
static GameScreen currentScreen = GameScreen::TITLE_SCREEN;

// Title screen input
static bool keyUp = false, keyDown = false;
static bool keyLeft = false, keyRight = false;
static bool keyEnter = false;

// ── HUD text rendering via bitmap quads ────────────────────────────────────
struct HudRenderer {
    unsigned int vao = 0, vbo = 0;
    Shader* shader = nullptr;

    void init() {
        glGenVertexArrays(1, &vao);
        glGenBuffers(1, &vbo);
    }

    void cleanup() {
        if (vao) { glDeleteVertexArrays(1, &vao); glDeleteBuffers(1, &vbo); }
    }

    void renderChar(std::vector<float>& verts, float x, float y, float w, float h,
                    char c, float r, float g, float b) {
        float t = h * 0.12f;
        float hw = w * 0.8f;
        float hh = h * 0.45f;

        auto hSeg = [&](float sx, float sy) {
            pushQuad(verts, sx, sy, sx + hw, sy + t, r, g, b);
        };
        auto vSeg = [&](float sx, float sy) {
            pushQuad(verts, sx, sy, sx + t, sy + hh, r, g, b);
        };

        float left = x;
        float right = x + hw - t;
        float bot = y;
        float mid = y + hh;
        float top = y + 2 * hh;

        if (c >= '0' && c <= '9') {
            bool segs[7] = {};
            switch (c) {
                case '0': segs[0]=segs[1]=segs[2]=segs[4]=segs[5]=segs[6]=true; break;
                case '1': segs[2]=segs[5]=true; break;
                case '2': segs[0]=segs[2]=segs[3]=segs[4]=segs[6]=true; break;
                case '3': segs[0]=segs[2]=segs[3]=segs[5]=segs[6]=true; break;
                case '4': segs[1]=segs[2]=segs[3]=segs[5]=true; break;
                case '5': segs[0]=segs[1]=segs[3]=segs[5]=segs[6]=true; break;
                case '6': segs[0]=segs[1]=segs[3]=segs[4]=segs[5]=segs[6]=true; break;
                case '7': segs[0]=segs[2]=segs[5]=true; break;
                case '8': segs[0]=segs[1]=segs[2]=segs[3]=segs[4]=segs[5]=segs[6]=true; break;
                case '9': segs[0]=segs[1]=segs[2]=segs[3]=segs[5]=segs[6]=true; break;
            }
            if (segs[0]) hSeg(left, top);
            if (segs[1]) vSeg(left, mid);
            if (segs[2]) vSeg(right, mid);
            if (segs[3]) hSeg(left, mid);
            if (segs[4]) vSeg(left, bot);
            if (segs[5]) vSeg(right, bot);
            if (segs[6]) hSeg(left, bot);
        } else if (c == ':') {
            float dotSize = t * 1.5f;
            float cx = x + hw * 0.4f;
            pushQuad(verts, cx, mid + hh * 0.3f, cx + dotSize, mid + hh * 0.3f + dotSize, r, g, b);
            pushQuad(verts, cx, bot + hh * 0.3f, cx + dotSize, bot + hh * 0.3f + dotSize, r, g, b);
        } else if (c == '/') {
            for (int i = 0; i < 5; i++) {
                float fx = x + hw * 0.2f + (hw * 0.6f) * i / 5.0f;
                float fy = bot + (top - bot) * i / 5.0f;
                pushQuad(verts, fx, fy, fx + t, fy + t * 2, r, g, b);
            }
        } else if (c == ' ') {
            // nothing
        } else {
            pushQuad(verts, x, bot, x + hw, top + t, r, g, b);
        }
    }

    void pushQuad(std::vector<float>& v, float x0, float y0, float x1, float y1,
                  float r, float g, float b) {
        v.insert(v.end(), {x0,y0, r,g,b});
        v.insert(v.end(), {x1,y0, r,g,b});
        v.insert(v.end(), {x1,y1, r,g,b});
        v.insert(v.end(), {x0,y0, r,g,b});
        v.insert(v.end(), {x1,y1, r,g,b});
        v.insert(v.end(), {x0,y1, r,g,b});
    }

    void renderText(Shader& hudShader, const std::string& text,
                    float x, float y, float charW, float charH,
                    float r, float g, float b, int scrW, int scrH) {
        std::vector<float> verts;
        float ndcCharW = charW / scrW * 2.0f;
        float ndcCharH = charH / scrH * 2.0f;
        float ndcX = (x / scrW) * 2.0f - 1.0f;
        float ndcY = (y / scrH) * 2.0f - 1.0f;

        for (size_t i = 0; i < text.size(); i++) {
            renderChar(verts, ndcX + i * ndcCharW * 1.2f, ndcY,
                       ndcCharW, ndcCharH, text[i], r, g, b);
        }

        if (verts.empty()) return;

        hudShader.use();
        glBindVertexArray(vao);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, verts.size() * sizeof(float), verts.data(), GL_DYNAMIC_DRAW);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 5*sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 5*sizeof(float), (void*)(2*sizeof(float)));
        glEnableVertexAttribArray(1);

        glDisable(GL_DEPTH_TEST);
        glDrawArrays(GL_TRIANGLES, 0, (int)(verts.size() / 5));
        glEnable(GL_DEPTH_TEST);
        glBindVertexArray(0);
    }

    // Render stars on the win screen (animated)
    void renderWinStars(Shader& hudShader, int earnedStars, float winElapsed,
                        std::vector<StarParticle>& particles,
                        int /*scrW*/, int scrH) {
        std::vector<float> verts;

        // Star positions: centered horizontally, above center
        float starY = 0.25f; // NDC Y
        float starSpacing = 0.15f;
        float startX = -starSpacing; // 3 stars centered

        // Pixel radii converted to NDC
        float outerR = 30.0f / scrH * 2.0f;
        float innerR = 13.0f / scrH * 2.0f;

        for (int i = 0; i < 3; i++) {
            float cx = startX + i * starSpacing;
            float cy = starY;

            float starAppearTime = 0.4f * i;
            float timeSinceAppear = winElapsed - starAppearTime;

            if (timeSinceAppear < 0.0f) continue; // not yet visible

            // Scale pop animation: starts at 200%, shrinks to 100% over 0.2s
            float scale = 1.0f;
            if (timeSinceAppear < 0.2f) {
                float t = timeSinceAppear / 0.2f;
                scale = 2.0f - t; // lerp from 2.0 to 1.0
            }

            // Spawn particles when star first appears
            if (timeSinceAppear < 0.017f) { // first frame
                for (int p = 0; p < 12; p++) {
                    float angle = p * 2.0f * (float)M_PI / 12.0f;
                    StarParticle sp;
                    sp.x = cx;
                    sp.y = cy;
                    sp.vx = cos(angle) * 0.3f;
                    sp.vy = sin(angle) * 0.3f;
                    sp.life = 0.5f;
                    sp.maxLife = 0.5f;
                    particles.push_back(sp);
                }
            }

            bool earned = (i < earnedStars);

            if (earned) {
                // Bright yellow #FFD700
                generateStarVerts(verts, cx, cy, outerR, innerR,
                                  1.0f, 0.843f, 0.0f, scale);
                // White outline
                generateStarOutline(verts, cx, cy, outerR, innerR,
                                    1.0f, 1.0f, 1.0f, scale);
            } else {
                // Dark gray #444444
                generateStarVerts(verts, cx, cy, outerR, innerR,
                                  0.267f, 0.267f, 0.267f, scale);
                // Dim gray outline
                generateStarOutline(verts, cx, cy, outerR, innerR,
                                    0.4f, 0.4f, 0.4f, scale);
            }
        }

        // Render particles
        for (auto& p : particles) {
            if (p.life <= 0.0f) continue;
            float alpha = p.life / p.maxLife;
            float pSize = 0.005f * alpha;
            pushQuad(verts, p.x - pSize, p.y - pSize, p.x + pSize, p.y + pSize,
                     1.0f * alpha, 0.843f * alpha, 0.0f);
        }

        if (verts.empty()) return;

        hudShader.use();
        glBindVertexArray(vao);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, verts.size() * sizeof(float), verts.data(), GL_DYNAMIC_DRAW);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 5*sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 5*sizeof(float), (void*)(2*sizeof(float)));
        glEnableVertexAttribArray(1);

        glDisable(GL_DEPTH_TEST);
        glDrawArrays(GL_TRIANGLES, 0, (int)(verts.size() / 5));
        glEnable(GL_DEPTH_TEST);
        glBindVertexArray(0);
    }

    // Render small inline stars for highscore/preview
    void renderSmallStars(Shader& hudShader, float x, float y, int earned, int total,
                          int scrW, int scrH) {
        std::vector<float> verts;

        float outerR = 8.0f / scrH * 2.0f;
        float innerR = 3.5f / scrH * 2.0f;
        float spacing = 20.0f / scrW * 2.0f;

        float ndcX = (x / scrW) * 2.0f - 1.0f;
        float ndcY = (y / scrH) * 2.0f - 1.0f;

        for (int i = 0; i < total; i++) {
            float cx = ndcX + i * spacing;
            float cy = ndcY;

            if (i < earned) {
                // Gold #FFD700
                generateStarVerts(verts, cx, cy, outerR, innerR,
                                  1.0f, 0.843f, 0.0f);
            } else {
                // Dark gray #444444
                generateStarVerts(verts, cx, cy, outerR, innerR,
                                  0.267f, 0.267f, 0.267f);
            }
        }

        if (verts.empty()) return;

        hudShader.use();
        glBindVertexArray(vao);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, verts.size() * sizeof(float), verts.data(), GL_DYNAMIC_DRAW);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 5*sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 5*sizeof(float), (void*)(2*sizeof(float)));
        glEnableVertexAttribArray(1);

        glDisable(GL_DEPTH_TEST);
        glDrawArrays(GL_TRIANGLES, 0, (int)(verts.size() / 5));
        glEnable(GL_DEPTH_TEST);
        glBindVertexArray(0);
    }
};

// ── Callbacks ──────────────────────────────────────────────────────────────
static Player* g_player = nullptr;

static void framebufferSizeCallback(GLFWwindow*, int width, int height) {
    screenWidth  = width;
    screenHeight = height;
    glViewport(0, 0, width, height);
}

static void mouseCallback(GLFWwindow*, double xposIn, double yposIn) {
    if (currentScreen != GameScreen::PLAYING) return;

    float xpos = (float)xposIn;
    float ypos = (float)yposIn;

    if (firstMouse) {
        lastMouseX = xpos;
        lastMouseY = ypos;
        firstMouse = false;
    }

    float xOff = xpos - lastMouseX;
    float yOff = lastMouseY - ypos;
    lastMouseX = xpos;
    lastMouseY = ypos;

    if (g_player) g_player->processMouseMovement(xOff, yOff);
}

static void keyCallback(GLFWwindow* window, int key, int /*scancode*/, int action, int /*mods*/) {
    bool pressed = (action == GLFW_PRESS || action == GLFW_REPEAT);
    bool down    = (action == GLFW_PRESS);

    // Navigation keys (work in all screens)
    switch (key) {
        case GLFW_KEY_UP:    keyUp = pressed; break;
        case GLFW_KEY_DOWN:  keyDown = pressed; break;
        case GLFW_KEY_LEFT:  keyLeft = pressed; break;
        case GLFW_KEY_RIGHT: keyRight = pressed; break;
        case GLFW_KEY_ENTER: keyEnter = pressed; break;
        default: break;
    }

    // WASD doubles as menu navigation on title screen
    bool onTitle = (currentScreen == GameScreen::TITLE_SCREEN);

    switch (key) {
        case GLFW_KEY_W: keyW = pressed; if (onTitle) keyUp = pressed; break;
        case GLFW_KEY_A: keyA = pressed; if (onTitle) keyLeft = pressed; break;
        case GLFW_KEY_S: keyS = pressed; if (onTitle) keyDown = pressed; break;
        case GLFW_KEY_D: keyD = pressed; if (onTitle) keyRight = pressed; break;
        case GLFW_KEY_SPACE: keySpace = pressed; if (onTitle) keyEnter = pressed; break;
        case GLFW_KEY_R:  if (down) requestRestart = true; break;
        case GLFW_KEY_F1: if (down) requestWireToggle = true; break;
        case GLFW_KEY_M:  if (down) requestMinimapToggle = true; break;
        case GLFW_KEY_L:  if (down) requestLegendToggle = true; break;
        case GLFW_KEY_T:  if (down) requestTorchToggle = true; break;
        case GLFW_KEY_ESCAPE:
            if (currentScreen == GameScreen::PLAYING) {
                currentScreen = GameScreen::TITLE_SCREEN;
            } else {
                glfwSetWindowShouldClose(window, true);
            }
            break;
        default: break;
    }
}

// ── Game state ─────────────────────────────────────────────────────────────
struct GameState {
    Maze maze;
    Player player;
    Renderer renderer;
    Minimap minimap;
    Collectible collectibles;
    glm::vec3 exitWorldPos;
    float elapsedTime;
    bool won;
    bool wireframe;
    Difficulty difficulty;

    // Win screen state
    float winScreenStartTime;
    StarResult starResult;
    std::vector<StarParticle> winParticles;
    bool scoreSaved;

    GameState(Difficulty diff)
        : maze(getDifficultyConfig(diff).mazeWidth, getDifficultyConfig(diff).mazeHeight),
          elapsedTime(0), won(false), wireframe(false), difficulty(diff),
          winScreenStartTime(0), scoreSaved(false)
    {
        starResult.stars = 0;
        starResult.perfectRun = false;
    }

    void restart(Difficulty diff) {
        difficulty = diff;
        auto cfg = getDifficultyConfig(diff);

        maze = Maze(cfg.mazeWidth, cfg.mazeHeight);
        maze.generate();

        auto [sx, sy] = maze.getStart();
        player.init((float)sx, (float)sy);

        renderer.buildMazeMesh(maze);

        auto [ex, ey] = maze.getExit();
        exitWorldPos = glm::vec3(ex * CELL_SIZE + CELL_SIZE * 0.5f,
                                 1.0f,
                                 ey * CELL_SIZE + CELL_SIZE * 0.5f);

        auto positions = maze.getItemPositions(cfg.numItems);
        collectibles.placeItems(positions);

        minimap.clearExplored();

        elapsedTime = 0.0f;
        won = false;
        winScreenStartTime = 0.0f;
        starResult.stars = 0;
        starResult.perfectRun = false;
        winParticles.clear();
        scoreSaved = false;
    }
};

// ── Main ───────────────────────────────────────────────────────────────────
int main() {
    // Initialize GLFW
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW\n";
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    GLFWwindow* window = glfwCreateWindow(screenWidth, screenHeight,
                                           "Maze Runner", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window\n";
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);
    glfwSetCursorPosCallback(window, mouseCallback);
    glfwSetKeyCallback(window, keyCallback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // Load OpenGL functions
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD\n";
        return -1;
    }

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    // Load shaders
    Shader mainShader("shaders/vertex.glsl", "shaders/fragment.glsl");
    Shader minimapShader("shaders/minimap_vertex.glsl", "shaders/minimap_fragment.glsl");
    Shader hudShader("shaders/hud_vertex.glsl", "shaders/hud_fragment.glsl");

    // Initialize game
    GameState game(currentDifficulty);
    game.renderer.init();
    game.minimap.init();
    game.restart(currentDifficulty);

    g_player = &game.player;

    HudRenderer hud;
    hud.init();

    TitleScreen titleScreen;
    titleScreen.init();

    HandRenderer handRenderer;
    handRenderer.init();

    SkyRenderer skyRenderer;
    skyRenderer.init();

    TorchLight torchLight;
    torchLight.init();

    // Load highscores for star preview
    auto highscores = loadHighscores(HIGHSCORE_FILE);

    float accumulator = 0.0f;
    float lastTime = (float)glfwGetTime();

    // ── Main loop ──────────────────────────────────────────────────────────
    while (!glfwWindowShouldClose(window)) {
        float currentTime = (float)glfwGetTime();
        float frameTime = currentTime - lastTime;
        lastTime = currentTime;
        if (frameTime > 0.25f) frameTime = 0.25f;

        // ── TITLE SCREEN ──────────────────────────────────────────────────
        if (currentScreen == GameScreen::TITLE_SCREEN) {
            // Show cursor on title screen
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

            bool startGame = titleScreen.update(frameTime,
                keyUp || keyW, keyDown || keyS,
                keyLeft || keyA, keyRight || keyD,
                keyEnter);

            if (titleScreen.shouldQuit()) {
                glfwSetWindowShouldClose(window, true);
                continue;
            }

            if (startGame) {
                currentDifficulty = titleScreen.getSelectedDifficulty();
                game.restart(currentDifficulty);
                highscores = loadHighscores(HIGHSCORE_FILE);
                currentScreen = GameScreen::PLAYING;
                firstMouse = true;
                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
                continue;
            }

            // Render title screen
            glClearColor(0.05f, 0.05f, 0.08f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            // Get best stars for each difficulty
            std::vector<int> bestStars;
            for (int d = 0; d < 4; d++) {
                bestStars.push_back(getBestStars(highscores, d));
            }

            hudShader.use();
            titleScreen.render(screenWidth, screenHeight, currentTime, bestStars);

            glfwSwapBuffers(window);
            glfwPollEvents();
            continue;
        }

        // ── PLAYING / WIN SCREEN ──────────────────────────────────────────

        // Handle toggle requests
        if (requestRestart) {
            game.restart(currentDifficulty);
            highscores = loadHighscores(HIGHSCORE_FILE);
            requestRestart = false;
        }
        if (requestWireToggle) {
            game.wireframe = !game.wireframe;
            game.renderer.setWireframe(game.wireframe);
            requestWireToggle = false;
        }
        if (requestMinimapToggle) {
            game.minimap.toggleVisible();
            requestMinimapToggle = false;
        }
        if (requestLegendToggle) {
            game.minimap.toggleLegend();
            requestLegendToggle = false;
        }
        if (requestTorchToggle) {
            torchLight.toggle();
            requestTorchToggle = false;
        }

        // Fixed timestep physics
        if (!game.won) {
            accumulator += frameTime;
            while (accumulator >= FIXED_DT) {
                game.player.update(FIXED_DT, game.maze,
                                   keyW, keyS, keyA, keyD, keySpace);
                game.collectibles.update(FIXED_DT);
                accumulator -= FIXED_DT;
            }
            game.elapsedTime += frameTime;
        }

        // Update sky and torch
        skyRenderer.update(frameTime);
        torchLight.update(frameTime);
        torchLight.setPlayerPosition(game.player.position, game.player.getFront());

        // Update explored cells for minimap
        {
            float pgx = game.player.position.x / CELL_SIZE;
            float pgy = game.player.position.z / CELL_SIZE;
            int cx = (int)std::floor(pgx);
            int cy = (int)std::floor(pgy);
            for (int dy = -3; dy <= 3; dy++) {
                for (int dx = -3; dx <= 3; dx++) {
                    int nx = cx + dx;
                    int ny = cy + dy;
                    if (nx >= 0 && nx < game.maze.getWidth() &&
                        ny >= 0 && ny < game.maze.getHeight())
                        game.minimap.markExplored(nx, ny);
                }
            }
        }

        // Try collecting items
        game.collectibles.tryCollect(game.player.position, 1.5f);

        // Check win condition
        if (!game.won && game.collectibles.allCollected()) {
            float dist = glm::length(game.player.position - game.exitWorldPos);
            if (dist < 2.0f) {
                game.won = true;
                game.winScreenStartTime = currentTime;

                // Calculate stars
                bool allCollected = game.collectibles.allCollected();
                game.starResult = calculateStars(currentDifficulty,
                                                 game.elapsedTime, allCollected);

                // Save highscore
                if (!game.scoreSaved) {
                    HighscoreEntry entry;
                    entry.name = "Player";
                    entry.score = std::max(1, (int)(10000.0f / (game.elapsedTime + 1.0f)));
                    entry.time = game.elapsedTime;
                    entry.difficulty = (int)currentDifficulty;
                    entry.collectables = game.collectibles.getCollectedCount();
                    entry.stars = game.starResult.stars;
                    entry.perfectRun = game.starResult.perfectRun ? 1 : 0;
                    addHighscore(HIGHSCORE_FILE, entry);
                    highscores = loadHighscores(HIGHSCORE_FILE);
                    game.scoreSaved = true;
                }
            }
        }

        // Update win screen particles
        if (game.won) {
            for (auto& p : game.winParticles) {
                p.x += p.vx * frameTime;
                p.y += p.vy * frameTime;
                p.life -= frameTime;
            }
            // Remove dead particles
            game.winParticles.erase(
                std::remove_if(game.winParticles.begin(), game.winParticles.end(),
                               [](const StarParticle& p) { return p.life <= 0.0f; }),
                game.winParticles.end());
        }

        // ── Render ─────────────────────────────────────────────────────────
        glm::vec3 fogCol = skyRenderer.getFogColor();
        glClearColor(fogCol.r, fogCol.g, fogCol.b, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        float aspect = (screenHeight > 0) ? (float)screenWidth / (float)screenHeight : 1.0f;
        glm::mat4 projection = glm::perspective(
            glm::radians(70.0f), aspect, 0.1f, 200.0f);
        glm::mat4 view = game.player.getViewMatrix();

        // Sky
        skyRenderer.render(view, projection);

        // Lighting parameters from sky
        glm::vec3 sunDir = skyRenderer.getSunDirection();
        glm::vec3 sunColor = skyRenderer.getSunColor();
        float ambientLevel = skyRenderer.getAmbientLevel();

        // Torch parameters
        bool torchOn = torchLight.isEnabled();
        glm::vec3 torchPos = torchLight.getPosition();
        glm::vec3 torchCol = torchLight.getColor();
        float torchRadius = torchLight.getRadius();

        // Maze
        game.renderer.renderMaze(mainShader, view, projection,
                                  sunDir, sunColor, ambientLevel, fogCol,
                                  torchOn, torchPos, torchCol, torchRadius);

        // Collectibles
        game.renderer.renderCollectibles(mainShader, view, projection,
                                          game.collectibles.getItems(),
                                          sunDir, sunColor, ambientLevel, fogCol,
                                          torchOn, torchPos, torchCol, torchRadius);

        // Exit portal
        game.renderer.renderExitPortal(mainShader, view, projection,
                                        game.exitWorldPos, currentTime,
                                        sunDir, sunColor, ambientLevel, fogCol,
                                        torchOn, torchPos, torchCol, torchRadius);

        // Torch glow sprite
        torchLight.renderGlow(mainShader, view, projection);

        // Update and render first-person hands
        {
            bool isMoving = keyW || keyA || keyS || keyD;
            bool isJumping = !game.player.isOnGround();
            bool isMovingBack = keyS && !keyW;
            handRenderer.update(frameTime, isMoving, isJumping, isMovingBack);
            handRenderer.render(mainShader, aspect);
        }

        // Minimap (with difficulty and dt for viewport scrolling)
        game.minimap.render(minimapShader, game.maze,
                            game.player.position, game.player.yaw,
                            game.collectibles.getItems(),
                            game.exitWorldPos,
                            screenWidth, screenHeight,
                            (int)currentDifficulty, frameTime);

        // HUD text
        {
            // Item counter
            char buf[64];
            std::snprintf(buf, sizeof(buf), "%d/%d",
                          game.collectibles.getCollectedCount(),
                          game.collectibles.getTotalCount());
            hud.renderText(hudShader, buf,
                           20, (float)screenHeight - 40, 16, 24,
                           1.0f, 0.84f, 0.0f,
                           screenWidth, screenHeight);

            // Timer
            int totalSec = (int)game.elapsedTime;
            int mins = totalSec / 60;
            int secs = totalSec % 60;
            std::snprintf(buf, sizeof(buf), "%02d:%02d", mins, secs);
            hud.renderText(hudShader, buf,
                           20, (float)screenHeight - 80, 16, 24,
                           1.0f, 1.0f, 1.0f,
                           screenWidth, screenHeight);

            // Difficulty indicator
            auto cfg = getDifficultyConfig(currentDifficulty);
            std::snprintf(buf, sizeof(buf), "%s", cfg.name);
            hud.renderText(hudShader, buf,
                           20, (float)screenHeight - 120, 12, 18,
                           0.6f, 0.6f, 0.8f,
                           screenWidth, screenHeight);

            // Star preview per difficulty (bottom-left)
            float previewY = 30.0f;
            for (int d = 0; d < 4; d++) {
                int bestStars = getBestStars(highscores, d);
                float px = 20.0f + d * 80.0f;
                hud.renderSmallStars(hudShader, px, previewY, bestStars, 3,
                                     screenWidth, screenHeight);
            }

            // Win screen
            if (game.won) {
                float winElapsed = currentTime - game.winScreenStartTime;

                // Timer display
                hud.renderText(hudShader, std::string(buf),
                               (float)screenWidth / 2 - 60,
                               (float)screenHeight / 2,
                               24, 36,
                               0.0f, 1.0f, 0.3f,
                               screenWidth, screenHeight);

                // Animated stars
                hud.renderWinStars(hudShader, game.starResult.stars, winElapsed,
                                   game.winParticles, screenWidth, screenHeight);

                // Perfect run indicator (golden star badge)
                if (game.starResult.perfectRun) {
                    hud.renderSmallStars(hudShader,
                                         (float)screenWidth / 2,
                                         (float)screenHeight / 2 + 40,
                                         1, 1,
                                         screenWidth, screenHeight);
                }

                // Score
                int score = std::max(1, (int)(10000.0f / (game.elapsedTime + 1.0f)));
                std::snprintf(buf, sizeof(buf), "%05d", score);
                hud.renderText(hudShader, buf,
                               (float)screenWidth / 2 - 50,
                               (float)screenHeight / 2 - 100,
                               20, 30,
                               1.0f, 1.0f, 1.0f,
                               screenWidth, screenHeight);

                // Return to title screen on R press from win screen
                if (requestRestart) {
                    currentScreen = GameScreen::TITLE_SCREEN;
                    requestRestart = false;
                }
            }
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    hud.cleanup();
    titleScreen.cleanup();
    handRenderer.cleanup();
    skyRenderer.cleanup();
    torchLight.cleanup();
    glfwTerminate();
    return 0;
}
