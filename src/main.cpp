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

#include <iostream>
#include <string>
#include <cmath>
#include <cstdio>

// ── Configuration ──────────────────────────────────────────────────────────
static const int MAZE_WIDTH  = 21;  // odd numbers for maze gen
static const int MAZE_HEIGHT = 21;
static const int NUM_ITEMS   = 7;
static const float CELL_SIZE = 2.0f;
static const float FIXED_DT  = 1.0f / 60.0f;

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

// ── HUD text rendering via bitmap quads ────────────────────────────────────
// Simple digit/character rendering using quads (no external font needed)
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

    // Render a simple rectangle-based character
    void renderChar(std::vector<float>& verts, float x, float y, float w, float h,
                    char c, float r, float g, float b) {
        // Segment display for digits and some characters
        // Each segment is a thin rectangle
        float t = h * 0.12f; // thickness
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
            //  _
            // |_|  7-segment style
            // |_|
            bool segs[7] = {}; // top, top-left, top-right, mid, bot-left, bot-right, bot
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
            if (segs[0]) hSeg(left, top);           // top
            if (segs[1]) vSeg(left, mid);            // top-left
            if (segs[2]) vSeg(right, mid);           // top-right
            if (segs[3]) hSeg(left, mid);            // middle
            if (segs[4]) vSeg(left, bot);            // bot-left
            if (segs[5]) vSeg(right, bot);           // bot-right
            if (segs[6]) hSeg(left, bot);            // bottom
        } else if (c == ':') {
            float dotSize = t * 1.5f;
            float cx = x + hw * 0.4f;
            pushQuad(verts, cx, mid + hh * 0.3f, cx + dotSize, mid + hh * 0.3f + dotSize, r, g, b);
            pushQuad(verts, cx, bot + hh * 0.3f, cx + dotSize, bot + hh * 0.3f + dotSize, r, g, b);
        } else if (c == '/') {
            // Simple diagonal approximation using small quads
            for (int i = 0; i < 5; i++) {
                float fx = x + hw * 0.2f + (hw * 0.6f) * i / 5.0f;
                float fy = bot + (top - bot) * i / 5.0f;
                pushQuad(verts, fx, fy, fx + t, fy + t * 2, r, g, b);
            }
        } else if (c == ' ') {
            // nothing
        } else {
            // For letters, just draw a filled block as fallback
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
        // Convert pixel coords to NDC
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
};

// ── Callbacks ──────────────────────────────────────────────────────────────
static Player* g_player = nullptr;

static void framebufferSizeCallback(GLFWwindow*, int width, int height) {
    screenWidth  = width;
    screenHeight = height;
    glViewport(0, 0, width, height);
}

static void mouseCallback(GLFWwindow*, double xposIn, double yposIn) {
    float xpos = (float)xposIn;
    float ypos = (float)yposIn;

    if (firstMouse) {
        lastMouseX = xpos;
        lastMouseY = ypos;
        firstMouse = false;
    }

    float xOff = xpos - lastMouseX;
    float yOff = lastMouseY - ypos; // reversed: y goes bottom to top
    lastMouseX = xpos;
    lastMouseY = ypos;

    if (g_player) g_player->processMouseMovement(xOff, yOff);
}

static void keyCallback(GLFWwindow* window, int key, int /*scancode*/, int action, int /*mods*/) {
    bool pressed = (action == GLFW_PRESS || action == GLFW_REPEAT);
    bool down    = (action == GLFW_PRESS);

    switch (key) {
        case GLFW_KEY_W: keyW = pressed; break;
        case GLFW_KEY_A: keyA = pressed; break;
        case GLFW_KEY_S: keyS = pressed; break;
        case GLFW_KEY_D: keyD = pressed; break;
        case GLFW_KEY_SPACE: keySpace = pressed; break;
        case GLFW_KEY_R:  if (down) requestRestart = true; break;
        case GLFW_KEY_F1: if (down) requestWireToggle = true; break;
        case GLFW_KEY_M:  if (down) requestMinimapToggle = true; break;
        case GLFW_KEY_ESCAPE: glfwSetWindowShouldClose(window, true); break;
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

    GameState() : maze(MAZE_WIDTH, MAZE_HEIGHT), elapsedTime(0), won(false), wireframe(false) {}

    void restart() {
        maze = Maze(MAZE_WIDTH, MAZE_HEIGHT);
        maze.generate();

        auto [sx, sy] = maze.getStart();
        player.init((float)sx, (float)sy);

        renderer.buildMazeMesh(maze);

        auto [ex, ey] = maze.getExit();
        exitWorldPos = glm::vec3(ex * CELL_SIZE + CELL_SIZE * 0.5f,
                                 1.0f,
                                 ey * CELL_SIZE + CELL_SIZE * 0.5f);

        auto positions = maze.getItemPositions(NUM_ITEMS);
        collectibles.placeItems(positions);

        minimap.clearExplored();

        elapsedTime = 0.0f;
        won = false;
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
    GameState game;
    game.renderer.init();
    game.minimap.init();
    game.restart();

    g_player = &game.player;

    HudRenderer hud;
    hud.init();

    float accumulator = 0.0f;
    float lastTime = (float)glfwGetTime();

    // ── Main loop ──────────────────────────────────────────────────────────
    while (!glfwWindowShouldClose(window)) {
        float currentTime = (float)glfwGetTime();
        float frameTime = currentTime - lastTime;
        lastTime = currentTime;
        if (frameTime > 0.25f) frameTime = 0.25f; // clamp

        // Handle toggle requests
        if (requestRestart) {
            game.restart();
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

        // Update explored cells for minimap (mark cells in a radius around player)
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
            }
        }

        // ── Render ─────────────────────────────────────────────────────────
        glClearColor(0.05f, 0.05f, 0.08f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glm::mat4 projection = glm::perspective(
            glm::radians(70.0f),
            (float)screenWidth / (float)screenHeight,
            0.1f, 200.0f);
        glm::mat4 view = game.player.getViewMatrix();

        // Maze
        game.renderer.renderMaze(mainShader, view, projection);

        // Collectibles
        game.renderer.renderCollectibles(mainShader, view, projection,
                                          game.collectibles.getItems());

        // Exit portal
        game.renderer.renderExitPortal(mainShader, view, projection,
                                        game.exitWorldPos, currentTime);

        // Minimap
        game.minimap.render(minimapShader, game.maze,
                            game.player.position, game.player.yaw,
                            game.collectibles.getItems(),
                            game.exitWorldPos,
                            screenWidth, screenHeight);

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

            // Win screen
            if (game.won) {
                std::snprintf(buf, sizeof(buf), "%02d:%02d", mins, secs);
                hud.renderText(hudShader, buf,
                               (float)screenWidth / 2 - 60,
                               (float)screenHeight / 2,
                               24, 36,
                               0.0f, 1.0f, 0.3f,
                               screenWidth, screenHeight);
            }
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    hud.cleanup();
    glfwTerminate();
    return 0;
}
