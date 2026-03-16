#include "Renderer.h"
#include "Maze.h"
#include "Collectible.h"
#include "Shader.h"
#include <glm/gtc/matrix_transform.hpp>
#include <cmath>
#include <iostream>
#include <string>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

static const float CELL_SIZE = 2.0f;
static const float WALL_HEIGHT = 4.0f;

// Graffiti texture filenames - add more by dropping files into textures/graffiti/
const std::vector<std::string> Renderer::GRAFFITI_FILES = {
    "graffiti_01.png",
    "graffiti_02.png",
    "graffiti_03.png",
    "graffiti_04.png",
	"graffiti_05.png"
};

Renderer::Renderer()
    : mazeVAO(0), mazeVBO(0), mazeVertexCount(0)
    , cubeVAO(0), cubeVBO(0), cubeVertexCount(0)
    , pyramidVAO(0), pyramidVBO(0), pyramidVertexCount(0)
    , sphereVAO(0), sphereVBO(0), sphereVertexCount(0)
    , giftBoxVAO(0), giftBoxVBO(0), giftBoxVertexCount(0), giftBoxTextureID(0)
    , wallTextureID(0)
    , graffitiVAO(0), graffitiVBO(0), graffitiVertexCount(0)
    , scratchVAO(0), scratchVBO(0)
    , playerScratchVAO(0), playerScratchVBO(0)
    , wireframe(false)
{
}

Renderer::~Renderer() {
    if (mazeVAO) { glDeleteVertexArrays(1, &mazeVAO); glDeleteBuffers(1, &mazeVBO); }
    if (cubeVAO) { glDeleteVertexArrays(1, &cubeVAO); glDeleteBuffers(1, &cubeVBO); }
    if (pyramidVAO) { glDeleteVertexArrays(1, &pyramidVAO); glDeleteBuffers(1, &pyramidVBO); }
    if (sphereVAO) { glDeleteVertexArrays(1, &sphereVAO); glDeleteBuffers(1, &sphereVBO); }
    if (giftBoxVAO) { glDeleteVertexArrays(1, &giftBoxVAO); glDeleteBuffers(1, &giftBoxVBO); }
    if (giftBoxTextureID) { glDeleteTextures(1, &giftBoxTextureID); }
    if (wallTextureID) { glDeleteTextures(1, &wallTextureID); }
    if (graffitiVAO) { glDeleteVertexArrays(1, &graffitiVAO); glDeleteBuffers(1, &graffitiVBO); }
    for (auto id : graffitiTextureIDs) {
        if (id) glDeleteTextures(1, &id);
    }
    if (scratchVAO) { glDeleteVertexArrays(1, &scratchVAO); glDeleteBuffers(1, &scratchVBO); }
    if (playerScratchVAO) { glDeleteVertexArrays(1, &playerScratchVAO); glDeleteBuffers(1, &playerScratchVBO); }
}

void Renderer::init() {
    generateWallTexture();
    generateGiftBoxTexture();
    generateGraffitiTextures();
    buildCubeMesh();
    buildPyramidMesh();
    buildSphereMesh();
    buildGiftBoxMesh();
    // Create scratch VAOs for cauldron and player rendering
    glGenVertexArrays(1, &scratchVAO);
    glGenBuffers(1, &scratchVBO);
    glGenVertexArrays(1, &playerScratchVAO);
    glGenBuffers(1, &playerScratchVBO);
}

void Renderer::setWireframe(bool enabled) {
    wireframe = enabled;
    glPolygonMode(GL_FRONT_AND_BACK, wireframe ? GL_LINE : GL_FILL);
}

// Helper: add a vertex (pos + color + normal)
static void pushVertex(std::vector<float>& v,
                       float px, float py, float pz,
                       float r, float g, float b,
                       float nx, float ny, float nz) {
    v.insert(v.end(), {px, py, pz, r, g, b, nx, ny, nz});
}

// Helper: add a quad (two triangles)
static void pushQuad(std::vector<float>& v,
                     float x0, float y0, float z0,
                     float x1, float y1, float z1,
                     float x2, float y2, float z2,
                     float x3, float y3, float z3,
                     float r, float g, float b,
                     float nx, float ny, float nz) {
    pushVertex(v, x0,y0,z0, r,g,b, nx,ny,nz);
    pushVertex(v, x1,y1,z1, r,g,b, nx,ny,nz);
    pushVertex(v, x2,y2,z2, r,g,b, nx,ny,nz);
    pushVertex(v, x2,y2,z2, r,g,b, nx,ny,nz);
    pushVertex(v, x3,y3,z3, r,g,b, nx,ny,nz);
    pushVertex(v, x0,y0,z0, r,g,b, nx,ny,nz);
}

// Helper: add a vertex (pos + color + normal + texcoord) for textured mesh
static void pushVertexUV(std::vector<float>& v,
                         float px, float py, float pz,
                         float r, float g, float b,
                         float nx, float ny, float nz,
                         float u, float tv) {
    v.insert(v.end(), {px, py, pz, r, g, b, nx, ny, nz, u, tv});
}

// Helper: add a textured quad (two triangles) with UV coordinates
static void pushQuadUV(std::vector<float>& v,
                       float x0, float y0, float z0,
                       float x1, float y1, float z1,
                       float x2, float y2, float z2,
                       float x3, float y3, float z3,
                       float r, float g, float b,
                       float nx, float ny, float nz,
                       float u0, float v0, float u1, float v1,
                       float u2, float v2, float u3, float v3) {
    pushVertexUV(v, x0,y0,z0, r,g,b, nx,ny,nz, u0,v0);
    pushVertexUV(v, x1,y1,z1, r,g,b, nx,ny,nz, u1,v1);
    pushVertexUV(v, x2,y2,z2, r,g,b, nx,ny,nz, u2,v2);
    pushVertexUV(v, x2,y2,z2, r,g,b, nx,ny,nz, u2,v2);
    pushVertexUV(v, x3,y3,z3, r,g,b, nx,ny,nz, u3,v3);
    pushVertexUV(v, x0,y0,z0, r,g,b, nx,ny,nz, u0,v0);
}

// Add a textured cube with UV mapping per face
static void addCubeTextured(std::vector<float>& verts,
                            float x, float y, float z,
                            float sx, float sy, float sz,
                            float r, float g, float b) {
    float x0 = x, x1 = x + sx;
    float y0 = y, y1 = y + sy;
    float z0 = z, z1 = z + sz;

    // UV tiling: 1 repeat per face
    float u0 = 0.0f, u1 = 1.0f;
    float tv0 = 0.0f, tv1 = 1.0f;

    // Front  (+Z)
    pushQuadUV(verts, x0,y0,z1, x1,y0,z1, x1,y1,z1, x0,y1,z1, r,g,b, 0,0,1,
               u0,tv0, u1,tv0, u1,tv1, u0,tv1);
    // Back   (-Z)
    pushQuadUV(verts, x1,y0,z0, x0,y0,z0, x0,y1,z0, x1,y1,z0, r,g,b, 0,0,-1,
               u0,tv0, u1,tv0, u1,tv1, u0,tv1);
    // Left   (-X)
    pushQuadUV(verts, x0,y0,z0, x0,y0,z1, x0,y1,z1, x0,y1,z0, r,g,b, -1,0,0,
               u0,tv0, u1,tv0, u1,tv1, u0,tv1);
    // Right  (+X)
    pushQuadUV(verts, x1,y0,z1, x1,y0,z0, x1,y1,z0, x1,y1,z1, r,g,b, 1,0,0,
               u0,tv0, u1,tv0, u1,tv1, u0,tv1);
    // Top    (+Y)
    pushQuadUV(verts, x0,y1,z1, x1,y1,z1, x1,y1,z0, x0,y1,z0, r,g,b, 0,1,0,
               u0,tv0, u1,tv0, u1,tv1, u0,tv1);
    // Bottom (-Y)
    pushQuadUV(verts, x0,y0,z0, x1,y0,z0, x1,y0,z1, x0,y0,z1, r,g,b, 0,-1,0,
               u0,tv0, u1,tv0, u1,tv1, u0,tv1);
}

void Renderer::addCube(std::vector<float>& verts,
                       float x, float y, float z,
                       float sx, float sy, float sz,
                       float r, float g, float b) {
    float x0 = x, x1 = x + sx;
    float y0 = y, y1 = y + sy;
    float z0 = z, z1 = z + sz;

    // Front  (+Z)
    pushQuad(verts, x0,y0,z1, x1,y0,z1, x1,y1,z1, x0,y1,z1, r,g,b, 0,0,1);
    // Back   (-Z)
    pushQuad(verts, x1,y0,z0, x0,y0,z0, x0,y1,z0, x1,y1,z0, r,g,b, 0,0,-1);
    // Left   (-X)
    pushQuad(verts, x0,y0,z0, x0,y0,z1, x0,y1,z1, x0,y1,z0, r,g,b, -1,0,0);
    // Right  (+X)
    pushQuad(verts, x1,y0,z1, x1,y0,z0, x1,y1,z0, x1,y1,z1, r,g,b, 1,0,0);
    // Top    (+Y)
    pushQuad(verts, x0,y1,z1, x1,y1,z1, x1,y1,z0, x0,y1,z0, r,g,b, 0,1,0);
    // Bottom (-Y)
    pushQuad(verts, x0,y0,z0, x1,y0,z0, x1,y0,z1, x0,y0,z1, r,g,b, 0,-1,0);
}

void Renderer::buildMazeMesh(const Maze& maze) {
    std::vector<float> verts;

    int w = maze.getWidth();
    int h = maze.getHeight();

    for (int y = 0; y < h; y++) {
        for (int x = 0; x < w; x++) {
            float wx = x * CELL_SIZE;
            float wz = y * CELL_SIZE;

            if (maze.isWall(x, y)) {
                // Minecraft stone brick walls with color variation
                unsigned int hash = (unsigned int)(x * 7919 + y * 104729);
                hash = ((hash >> 16) ^ hash) * 0x45d9f3b;
                hash = (hash >> 16) ^ hash;
                float variation = (float)(hash % 100) / 500.0f;

                // Choose between stone brick variants
                float wr, wg, wb;
                int variant = hash % 5;
                if (variant == 0) {
                    // Mossy stone brick (greenish)
                    wr = 0.35f + variation; wg = 0.42f + variation; wb = 0.30f + variation;
                } else if (variant == 1) {
                    // Cobblestone (lighter gray)
                    wr = 0.50f + variation; wg = 0.48f + variation; wb = 0.45f + variation;
                } else {
                    // Standard stone brick
                    wr = 0.45f + variation; wg = 0.43f + variation; wb = 0.40f + variation;
                }
                addCubeTextured(verts, wx, 0.0f, wz, CELL_SIZE, WALL_HEIGHT, CELL_SIZE,
                        wr, wg, wb);
            } else {
                // Minecraft grass block floor
                unsigned int fhash = (unsigned int)(x * 3571 + y * 7907);
                fhash = ((fhash >> 16) ^ fhash) * 0x45d9f3b;
                fhash = (fhash >> 16) ^ fhash;
                float fvar = (float)(fhash % 100) / 600.0f;
                // Grass green color with variation
                float fr = 0.28f + fvar * 0.5f;
                float fg = 0.45f + fvar;
                float fb = 0.18f + fvar * 0.3f;

                // Floor (textured)
                pushQuadUV(verts,
                    wx,         0.0f, wz,
                    wx+CELL_SIZE, 0.0f, wz,
                    wx+CELL_SIZE, 0.0f, wz+CELL_SIZE,
                    wx,         0.0f, wz+CELL_SIZE,
                    fr, fg, fb,
                    0.0f, 1.0f, 0.0f,
                    0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f);
                // Ceiling (dark stone, textured)
                pushQuadUV(verts,
                    wx,         WALL_HEIGHT, wz+CELL_SIZE,
                    wx+CELL_SIZE, WALL_HEIGHT, wz+CELL_SIZE,
                    wx+CELL_SIZE, WALL_HEIGHT, wz,
                    wx,         WALL_HEIGHT, wz,
                    0.12f, 0.12f, 0.15f,
                    0.0f, -1.0f, 0.0f,
                    0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f);
            }
        }
    }

    // Stride is 11 floats: pos(3) + color(3) + normal(3) + texcoord(2)
    mazeVertexCount = (int)(verts.size() / 11);

    if (mazeVAO) { glDeleteVertexArrays(1, &mazeVAO); glDeleteBuffers(1, &mazeVBO); }

    glGenVertexArrays(1, &mazeVAO);
    glGenBuffers(1, &mazeVBO);

    glBindVertexArray(mazeVAO);
    glBindBuffer(GL_ARRAY_BUFFER, mazeVBO);
    glBufferData(GL_ARRAY_BUFFER, verts.size() * sizeof(float), verts.data(), GL_STATIC_DRAW);

    // Position
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // Color
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    // Normal
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);
    // TexCoord
    glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)(9 * sizeof(float)));
    glEnableVertexAttribArray(3);

    glBindVertexArray(0);

    // Build graffiti decals on maze walls
    buildGraffitiMesh(maze);
}

void Renderer::generateWallTexture() {
    glGenTextures(1, &wallTextureID);
    glBindTexture(GL_TEXTURE_2D, wallTextureID);

    // Try loading an external texture from textures/wall.png
    int imgW, imgH, imgChannels;
    unsigned char* data = stbi_load("textures/wall.png", &imgW, &imgH, &imgChannels, 3);
    if (data) {
        std::cerr << "Loaded wall texture from textures/wall.png ("
                  << imgW << "x" << imgH << ")\n";
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, imgW, imgH, 0,
                     GL_RGB, GL_UNSIGNED_BYTE, data);
        stbi_image_free(data);
    } else {
        std::cerr << "textures/wall.png not found, using procedural texture\n";

        // Fall back to procedural stone brick texture
        const int TEX_SIZE = 128;
        std::vector<unsigned char> pixels(TEX_SIZE * TEX_SIZE * 3);

        const int brickH = TEX_SIZE / 4;
        const int brickW = TEX_SIZE / 2;
        const int mortarSize = 2;

        for (int y = 0; y < TEX_SIZE; y++) {
            for (int x = 0; x < TEX_SIZE; x++) {
                int row = y / brickH;
                int offsetX = (row % 2 == 1) ? brickW / 2 : 0;
                int localX = (x + offsetX) % TEX_SIZE;
                int brickCol = localX / brickW;
                int inBrickX = localX % brickW;
                int inBrickY = y % brickH;

                bool isMortar = (inBrickX < mortarSize || inBrickY < mortarSize);

                int idx = (y * TEX_SIZE + x) * 3;

                if (isMortar) {
                    pixels[idx + 0] = 60;
                    pixels[idx + 1] = 58;
                    pixels[idx + 2] = 55;
                } else {
                    unsigned int bHash = (unsigned int)(row * 1237 + brickCol * 4391 + 7);
                    bHash = ((bHash >> 16) ^ bHash) * 0x45d9f3b;
                    bHash = (bHash >> 16) ^ bHash;
                    int bVar = (int)(bHash % 30) - 15;

                    unsigned int pHash = (unsigned int)(x * 131 + y * 997);
                    pHash = ((pHash >> 16) ^ pHash) * 0x45d9f3b;
                    pHash = (pHash >> 16) ^ pHash;
                    int pNoise = (int)(pHash % 20) - 10;

                    int base_r = 140 + bVar + pNoise;
                    int base_g = 135 + bVar + pNoise;
                    int base_b = 125 + bVar + pNoise;

                    pixels[idx + 0] = (unsigned char)(base_r < 0 ? 0 : (base_r > 255 ? 255 : base_r));
                    pixels[idx + 1] = (unsigned char)(base_g < 0 ? 0 : (base_g > 255 ? 255 : base_g));
                    pixels[idx + 2] = (unsigned char)(base_b < 0 ? 0 : (base_b > 255 ? 255 : base_b));
                }
            }
        }

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, TEX_SIZE, TEX_SIZE, 0,
                     GL_RGB, GL_UNSIGNED_BYTE, pixels.data());
    }

    glGenerateMipmap(GL_TEXTURE_2D);

    // Use nearest-neighbor filtering for pixelated Minecraft look
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glBindTexture(GL_TEXTURE_2D, 0);
}

void Renderer::generateGiftBoxTexture() {
    glGenTextures(1, &giftBoxTextureID);
    glBindTexture(GL_TEXTURE_2D, giftBoxTextureID);

    // Try loading an external texture from textures/gift_box.png
    int imgW, imgH, imgChannels;
    unsigned char* data = stbi_load("textures/gift_box.png", &imgW, &imgH, &imgChannels, 4);
    if (data) {
        std::cerr << "Loaded gift box texture from textures/gift_box.png ("
                  << imgW << "x" << imgH << ")\n";
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, imgW, imgH, 0,
                     GL_RGBA, GL_UNSIGNED_BYTE, data);
        stbi_image_free(data);
    } else {
        std::cerr << "textures/gift_box.png not found, using procedural texture\n";

        // Procedural gift box texture: red box with gold ribbon cross
        const int TEX_SIZE = 64;
        std::vector<unsigned char> pixels(TEX_SIZE * TEX_SIZE * 4);

        const int ribbonWidth = 3;
        const int center = TEX_SIZE / 2;

        for (int y = 0; y < TEX_SIZE; y++) {
            for (int x = 0; x < TEX_SIZE; x++) {
                int idx = (y * TEX_SIZE + x) * 4;
                unsigned char r = 200, g = 50, b = 50, a = 255;

                // Highlight on top third
                if (y < TEX_SIZE / 3) { r = 230; g = 80; b = 80; }
                // Shadow on bottom third
                else if (y > TEX_SIZE * 2 / 3) { r = 150; g = 35; b = 35; }

                // Gold ribbon: horizontal band
                if (std::abs(y - center) <= ribbonWidth) { r = 255; g = 215; b = 0; }
                // Gold ribbon: vertical band
                if (std::abs(x - center) <= ribbonWidth) { r = 255; g = 215; b = 0; }

                // Bow at center intersection
                int dx = x - center;
                int dy = y - center;
                if (std::abs(dx) < ribbonWidth * 3 && std::abs(dy) < ribbonWidth * 2) {
                    if (std::abs(dx) + (int)(std::abs(dy) * 1.5f) < ribbonWidth * 4) {
                        r = 255; g = 235; b = 50;
                    }
                }

                // Dark border
                if (x <= 1 || x >= TEX_SIZE - 2 || y <= 1 || y >= TEX_SIZE - 2) {
                    r = 100; g = 25; b = 25;
                }

                pixels[idx + 0] = r;
                pixels[idx + 1] = g;
                pixels[idx + 2] = b;
                pixels[idx + 3] = a;
            }
        }

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, TEX_SIZE, TEX_SIZE, 0,
                     GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());
    }

    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void Renderer::buildGiftBoxMesh() {
    // Textured unit cube from (-0.5,-0.5,-0.5) to (0.5,0.5,0.5)
    // Vertex format: pos(3) + color(3) + normal(3) + texcoord(2) = 11 floats
    std::vector<float> verts;
    addCubeTextured(verts, -0.5f, -0.5f, -0.5f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f);
    giftBoxVertexCount = (int)(verts.size() / 11);

    glGenVertexArrays(1, &giftBoxVAO);
    glGenBuffers(1, &giftBoxVBO);
    glBindVertexArray(giftBoxVAO);
    glBindBuffer(GL_ARRAY_BUFFER, giftBoxVBO);
    glBufferData(GL_ARRAY_BUFFER, verts.size() * sizeof(float), verts.data(), GL_STATIC_DRAW);

    // Same vertex layout as maze mesh (11-float stride)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)(9 * sizeof(float)));
    glEnableVertexAttribArray(3);

    glBindVertexArray(0);
}
void Renderer::generateGraffitiTextures() {
    // Try loading external graffiti textures from textures/graffiti/
    for (const auto& filename : GRAFFITI_FILES) {
        std::string path = "textures/graffiti/" + filename;
        int imgW, imgH, imgChannels;
        unsigned char* data = stbi_load(path.c_str(), &imgW, &imgH, &imgChannels, 4);
        if (data) {
            unsigned int texID;
            glGenTextures(1, &texID);
            glBindTexture(GL_TEXTURE_2D, texID);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, imgW, imgH, 0,
                         GL_RGBA, GL_UNSIGNED_BYTE, data);
            glGenerateMipmap(GL_TEXTURE_2D);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            glBindTexture(GL_TEXTURE_2D, 0);
            graffitiTextureIDs.push_back(texID);
            stbi_image_free(data);
            std::cerr << "Loaded graffiti: " << path << "\n";
        }
    }

    // If no external textures loaded, generate procedural graffiti
    if (graffitiTextureIDs.empty()) {
        std::cerr << "No graffiti files found, generating procedural graffiti\n";
        const int TEX_SIZE = 64;

        // Procedural graffiti patterns (pixel art style)
        // Pattern 1: Arrow symbol
        // Pattern 2: X mark
        // Pattern 3: Spiral/circle
        // Pattern 4: Blocky skull

        struct PatternDef {
            float r, g, b;
            // Callback to determine if pixel (x,y) is filled
        };

        // Simple lambda-based pattern definitions
        auto genTexture = [&](auto patternFunc, float cr, float cg, float cb) {
            std::vector<unsigned char> pixels(TEX_SIZE * TEX_SIZE * 4, 0);
            for (int y = 0; y < TEX_SIZE; y++) {
                for (int x = 0; x < TEX_SIZE; x++) {
                    int idx = (y * TEX_SIZE + x) * 4;
                    if (patternFunc(x, y, TEX_SIZE)) {
                        // Add slight noise for worn look
                        unsigned int hash = (unsigned int)(x * 7919 + y * 104729);
                        hash = ((hash >> 16) ^ hash) * 0x45d9f3b;
                        float noise = (float)(hash % 60) / 255.0f - 0.12f;
                        pixels[idx + 0] = (unsigned char)(std::max(0.0f, std::min(255.0f, cr * 255.0f + noise * 255.0f)));
                        pixels[idx + 1] = (unsigned char)(std::max(0.0f, std::min(255.0f, cg * 255.0f + noise * 255.0f)));
                        pixels[idx + 2] = (unsigned char)(std::max(0.0f, std::min(255.0f, cb * 255.0f + noise * 255.0f)));
                        pixels[idx + 3] = 220; // mostly opaque
                    }
                }
            }
            unsigned int texID;
            glGenTextures(1, &texID);
            glBindTexture(GL_TEXTURE_2D, texID);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, TEX_SIZE, TEX_SIZE, 0,
                         GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());
            glGenerateMipmap(GL_TEXTURE_2D);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            glBindTexture(GL_TEXTURE_2D, 0);
            graffitiTextureIDs.push_back(texID);
        };

        // Pattern 1: Arrow pointing right (red)
        genTexture([](int x, int y, int s) {
            int cx = s / 2, cy = s / 2;
            int bx = x / 4, by = y / 4;
            int bcx = cx / 4, bcy = cy / 4;
            // Horizontal bar
            if (by == bcy && bx >= bcx - 4 && bx <= bcx + 2) return true;
            // Arrow head
            if (bx >= bcx + 2 && bx <= bcx + 5) {
                int dist = bx - bcx - 2;
                if (std::abs(by - bcy) <= (3 - dist) && (3 - dist) >= 0) return true;
            }
            return false;
        }, 0.9f, 0.15f, 0.1f);

        // Pattern 2: X mark (yellow-orange)
        genTexture([](int x, int y, int s) {
            int bx = x / 4, by = y / 4;
            int bs = s / 4;
            int margin = 2;
            if (bx >= margin && bx < bs - margin && by >= margin && by < bs - margin) {
                int lx = bx - margin, ly = by - margin;
                int range = bs - 2 * margin;
                // Two diagonals
                if (std::abs(lx - ly) <= 1) return true;
                if (std::abs(lx - (range - 1 - ly)) <= 1) return true;
            }
            return false;
        }, 1.0f, 0.7f, 0.1f);

        // Pattern 3: Circle/ring (cyan)
        genTexture([](int x, int y, int s) {
            float cx = s / 2.0f, cy = s / 2.0f;
            float dist = std::sqrt((x - cx) * (x - cx) + (y - cy) * (y - cy));
            float r = s * 0.35f;
            float thickness = s * 0.08f;
            if (dist > r - thickness && dist < r + thickness) return true;
            return false;
        }, 0.1f, 0.9f, 0.85f);

        // Pattern 4: Blocky creeper face (green)
        genTexture([](int x, int y, int s) {
            int bx = x / 4, by = y / 4;
            int bs = s / 4;
            int cx = bs / 2;
            // Eyes (two 2x2 blocks)
            if ((bx >= cx - 4 && bx <= cx - 2 && by >= 3 && by <= 5) ||
                (bx >= cx + 1 && bx <= cx + 3 && by >= 3 && by <= 5))
                return true;
            // Nose/mouth area
            if (bx >= cx - 1 && bx <= cx && by >= 6 && by <= 7)
                return true;
            if (bx >= cx - 3 && bx <= cx + 2 && by >= 8 && by <= 9)
                return true;
            if ((bx >= cx - 3 && bx <= cx - 2 && by >= 10 && by <= 12) ||
                (bx >= cx + 1 && bx <= cx + 2 && by >= 10 && by <= 12))
                return true;
            return false;
        }, 0.1f, 0.7f, 0.1f);
    }
}

void Renderer::buildGraffitiMesh(const Maze& maze) {
    if (graffitiTextureIDs.empty()) return;

    // We need separate draw calls per texture, but for simplicity we'll
    // store per-vertex texture index in color channel and render all graffiti
    // with the same texture in batches. For this implementation, we batch
    // all graffiti into one VBO with texture index encoded, then render
    // in passes per texture.

    // Actually, simpler: store all graffiti quads in one mesh and track
    // which texture each set of 6 vertices uses. We'll render in passes.

    // Vertex format: pos(3) + normal(3) + texcoord(2) = 8 floats
    struct GraffitiBatch {
        std::vector<float> verts;
    };

    std::vector<GraffitiBatch> batches(graffitiTextureIDs.size());

    int w = maze.getWidth();
    int h = maze.getHeight();

    // Simple hash-based pseudo-random for deterministic graffiti placement
    auto hashCell = [](int x, int y, int face) -> unsigned int {
        unsigned int h = (unsigned int)(x * 73856093 ^ y * 19349663 ^ face * 83492791);
        h = ((h >> 16) ^ h) * 0x45d9f3b;
        h = (h >> 16) ^ h;
        return h;
    };

    for (int y = 0; y < h; y++) {
        for (int x = 0; x < w; x++) {
            if (!maze.isWall(x, y)) continue;

            float wx = x * CELL_SIZE;
            float wz = y * CELL_SIZE;

            // Check each face: only place graffiti on faces adjacent to path cells
            // Face 0: -Z (back face, adjacent to y-1)
            // Face 1: +Z (front face, adjacent to y+1)
            // Face 2: -X (left face, adjacent to x-1)
            // Face 3: +X (right face, adjacent to x+1)

            struct FaceInfo {
                int adjX, adjY;      // adjacent cell
                float nx, ny, nz;    // inward normal (toward path)
                // Quad corners (bottom-left, bottom-right, top-right, top-left)
                float x0, py0, z0, x1, py1, z1, x2, py2, z2, x3, py3, z3;
            };

            FaceInfo faces[4] = {
                // -Z face (wall at y-1 side), normal pointing toward -Z path
                {x, y - 1,  0, 0, -1,
                 wx + CELL_SIZE, 0.0f, wz,  wx, 0.0f, wz,
                 wx, WALL_HEIGHT, wz,  wx + CELL_SIZE, WALL_HEIGHT, wz},
                // +Z face (wall at y+1 side), normal pointing toward +Z path
                {x, y + 1,  0, 0, 1,
                 wx, 0.0f, wz + CELL_SIZE,  wx + CELL_SIZE, 0.0f, wz + CELL_SIZE,
                 wx + CELL_SIZE, WALL_HEIGHT, wz + CELL_SIZE,  wx, WALL_HEIGHT, wz + CELL_SIZE},
                // -X face (wall at x-1 side), normal pointing toward -X path
                {x - 1, y,  -1, 0, 0,
                 wx, 0.0f, wz,  wx, 0.0f, wz + CELL_SIZE,
                 wx, WALL_HEIGHT, wz + CELL_SIZE,  wx, WALL_HEIGHT, wz},
                // +X face (wall at x+1 side), normal pointing toward +X path
                {x + 1, y,  1, 0, 0,
                 wx + CELL_SIZE, 0.0f, wz + CELL_SIZE,  wx + CELL_SIZE, 0.0f, wz,
                 wx + CELL_SIZE, WALL_HEIGHT, wz,  wx + CELL_SIZE, WALL_HEIGHT, wz + CELL_SIZE},
            };

            for (int f = 0; f < 4; f++) {
                auto& face = faces[f];

                // Check if adjacent cell is a path (not wall and in bounds)
                if (face.adjX < 0 || face.adjX >= w || face.adjY < 0 || face.adjY >= h)
                    continue;
                if (maze.isWall(face.adjX, face.adjY))
                    continue;

                // Decide if this face gets graffiti
                unsigned int rh = hashCell(x, y, f);
                float randomValue = (float)(rh % 1000) / 1000.0f;
                if (randomValue > GRAFFITI_CHANCE) continue;

                // Select graffiti texture
                int texIdx = (int)(rh / 1000 % graffitiTextureIDs.size());

                // Random scale (0.5x to 1.5x of wall height, clamped to wall)
                unsigned int rh2 = hashCell(x + 100, y + 200, f + 300);
                float scale = 0.5f + (float)(rh2 % 100) / 100.0f; // 0.5 to 1.5
                float grafH = WALL_HEIGHT * scale * 0.5f;
                float grafW = CELL_SIZE * scale * 0.5f;
                grafH = std::min(grafH, WALL_HEIGHT * 0.9f);
                grafW = std::min(grafW, CELL_SIZE * 0.9f);

                // Random position offset within face
                float maxOffH = (WALL_HEIGHT - grafH) * 0.5f;
                float maxOffW = (CELL_SIZE - grafW) * 0.5f;
                float offH = maxOffH * ((float)(rh2 / 100 % 100) / 100.0f * 2.0f - 1.0f) * 0.5f;
                float offW = maxOffW * ((float)(rh2 / 10000 % 100) / 100.0f * 2.0f - 1.0f) * 0.5f;

                // Center of the wall face
                float cX = (face.x0 + face.x1 + face.x2 + face.x3) * 0.25f;
                float cY = WALL_HEIGHT * 0.5f + offH;
                float cZ = (face.z0 + face.z1 + face.z2 + face.z3) * 0.25f;

                // Offset slightly from wall to avoid z-fighting
                float offsetDist = 0.01f;
                float oX = face.nx * offsetDist;
                float oZ = face.nz * offsetDist;

                // UV rotation: sometimes flip 180
                bool flipUV = ((rh2 / 1000000) % 2 == 1);
                float u0 = flipUV ? 1.0f : 0.0f;
                float u1 = flipUV ? 0.0f : 1.0f;
                float v0 = 0.0f, v1 = 1.0f;

                // Build the graffiti quad
                auto& batch = batches[texIdx];

                if (face.nz != 0) {
                    // Face on Z plane - graffiti extends in X and Y
                    float hw = grafW * 0.5f;
                    float hh = grafH * 0.5f;
                    float px = cX + offW + oX;
                    float pz = cZ + oZ;

                    // Triangle 1
                    batch.verts.insert(batch.verts.end(), {px - hw, cY - hh, pz, face.nx, face.ny, face.nz, u0, v0});
                    batch.verts.insert(batch.verts.end(), {px + hw, cY - hh, pz, face.nx, face.ny, face.nz, u1, v0});
                    batch.verts.insert(batch.verts.end(), {px + hw, cY + hh, pz, face.nx, face.ny, face.nz, u1, v1});
                    // Triangle 2
                    batch.verts.insert(batch.verts.end(), {px - hw, cY - hh, pz, face.nx, face.ny, face.nz, u0, v0});
                    batch.verts.insert(batch.verts.end(), {px + hw, cY + hh, pz, face.nx, face.ny, face.nz, u1, v1});
                    batch.verts.insert(batch.verts.end(), {px - hw, cY + hh, pz, face.nx, face.ny, face.nz, u0, v1});
                } else {
                    // Face on X plane - graffiti extends in Z and Y
                    float hw = grafW * 0.5f;
                    float hh = grafH * 0.5f;
                    float px = cX + oX;
                    float pz = cZ + offW + oZ;

                    // Triangle 1
                    batch.verts.insert(batch.verts.end(), {px, cY - hh, pz - hw, face.nx, face.ny, face.nz, u0, v0});
                    batch.verts.insert(batch.verts.end(), {px, cY - hh, pz + hw, face.nx, face.ny, face.nz, u1, v0});
                    batch.verts.insert(batch.verts.end(), {px, cY + hh, pz + hw, face.nx, face.ny, face.nz, u1, v1});
                    // Triangle 2
                    batch.verts.insert(batch.verts.end(), {px, cY - hh, pz - hw, face.nx, face.ny, face.nz, u0, v0});
                    batch.verts.insert(batch.verts.end(), {px, cY + hh, pz + hw, face.nx, face.ny, face.nz, u1, v1});
                    batch.verts.insert(batch.verts.end(), {px, cY + hh, pz - hw, face.nx, face.ny, face.nz, u0, v1});
                }
            }
        }
    }

    // Merge all batches into a single VBO with batch offsets
    // For simplicity, we store graffiti batch info and render in passes
    // But first, let's use a simpler approach: interleave texture index info
    // Since OpenGL doesn't support per-vertex texture binding, we render in passes

    // Store batch start/count for rendering
    std::vector<float> allVerts;
    // We'll track batch boundaries using graffitiVertexCount as total
    // and store batch info in a member. For minimal changes, let's just
    // flatten and use the first texture for all graffiti, OR render per-batch.

    // Actually the cleanest approach: store each batch separately in the VBO
    // and record offsets. We'll store them sequentially.

    // We need a helper struct. For minimal additions, store counts per batch.
    // Let's use a simple approach: total verts, render all with same settings.

    // Store batch info as a concatenated buffer with known offsets
    struct BatchInfo {
        int startVertex;
        int vertexCount;
        int textureIdx;
    };
    std::vector<BatchInfo> batchInfos;

    for (size_t i = 0; i < batches.size(); i++) {
        if (batches[i].verts.empty()) continue;
        BatchInfo bi;
        bi.startVertex = (int)(allVerts.size() / 8);
        bi.vertexCount = (int)(batches[i].verts.size() / 8);
        bi.textureIdx = (int)i;
        batchInfos.push_back(bi);
        allVerts.insert(allVerts.end(), batches[i].verts.begin(), batches[i].verts.end());
    }

    graffitiVertexCount = (int)(allVerts.size() / 8);

    if (graffitiVertexCount == 0) return;

    if (graffitiVAO) { glDeleteVertexArrays(1, &graffitiVAO); glDeleteBuffers(1, &graffitiVBO); }

    glGenVertexArrays(1, &graffitiVAO);
    glGenBuffers(1, &graffitiVBO);
    glBindVertexArray(graffitiVAO);
    glBindBuffer(GL_ARRAY_BUFFER, graffitiVBO);
    glBufferData(GL_ARRAY_BUFFER, allVerts.size() * sizeof(float), allVerts.data(), GL_STATIC_DRAW);

    // Stride: 8 floats (pos3 + normal3 + uv2)
    // Position
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // Normal (reuse color slot for shading)
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(2);
    // TexCoord
    glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(3);

    glBindVertexArray(0);

    // Store batch rendering info
    graffitiBatchInfos_.clear();
    for (auto& bi : batchInfos) {
        GraffitiBatchInfo gbi;
        gbi.startVertex = bi.startVertex;
        gbi.vertexCount = bi.vertexCount;
        gbi.textureIdx = bi.textureIdx;
        graffitiBatchInfos_.push_back(gbi);
    }
}
static void setLightingUniforms(Shader& shader,
                                const glm::vec3& sunDir, const glm::vec3& sunColor,
                                float ambientLevel, const glm::vec3& fogCol,
                                bool torchEnabled, const glm::vec3& torchPos,
                                const glm::vec3& torchColor, float torchRadius) {
    shader.setVec3("lightDir", sunDir);
    shader.setVec3("fogColor", fogCol);
    shader.setFloat("fogDensity", 0.035f);
    shader.setFloat("fogGradient", 2.0f);
    shader.setVec3("sunColor", sunColor);
    shader.setFloat("ambientLevel", ambientLevel);
    shader.setBool("torchEnabled", torchEnabled);
    shader.setVec3("torchPos", torchPos);
    shader.setVec3("torchColor", torchColor);
    shader.setFloat("torchRadius", torchRadius);
    shader.setBool("enableEdgeOutline", true);
}

void Renderer::renderMaze(Shader& shader, const glm::mat4& view, const glm::mat4& projection,
                           const glm::vec3& sunDir, const glm::vec3& sunColor,
                           float ambientLevel, const glm::vec3& fogCol,
                           bool torchEnabled, const glm::vec3& torchPos,
                           const glm::vec3& torchColor, float torchRadius) {
    shader.use();
    glm::mat4 model = glm::mat4(1.0f);
    shader.setMat4("model", model);
    shader.setMat4("view", view);
    shader.setMat4("projection", projection);

    setLightingUniforms(shader, sunDir, sunColor, ambientLevel, fogCol,
                        torchEnabled, torchPos, torchColor, torchRadius);

    // Bind wall texture
    shader.setBool("useTexture", true);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, wallTextureID);
    shader.setInt("wallTexture", 0);

    glBindVertexArray(mazeVAO);
    glDrawArrays(GL_TRIANGLES, 0, mazeVertexCount);
    glBindVertexArray(0);

    // Render graffiti decals
    if (graffitiVAO && !graffitiBatchInfos_.empty()) {
        shader.setBool("useTexture", true);
        shader.setBool("enableEdgeOutline", false);
        // Set vertex color to white so texture shows through properly
        // The graffiti VAO has no color attribute, so we set a default
        glVertexAttrib3f(1, 1.0f, 1.0f, 1.0f);

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glDepthMask(GL_FALSE); // don't write to depth for decals

        glBindVertexArray(graffitiVAO);
        for (auto& bi : graffitiBatchInfos_) {
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, graffitiTextureIDs[bi.textureIdx]);
            shader.setInt("wallTexture", 0);
            glDrawArrays(GL_TRIANGLES, bi.startVertex, bi.vertexCount);
        }
        glBindVertexArray(0);

        glDepthMask(GL_TRUE);
        glDisable(GL_BLEND);
        shader.setBool("enableEdgeOutline", true);
    }
}

void Renderer::buildCubeMesh() {
    std::vector<float> verts;
    addCube(verts, -0.5f, -0.5f, -0.5f, 1.0f, 1.0f, 1.0f, 1.0f, 0.84f, 0.0f);
    cubeVertexCount = (int)(verts.size() / 9);

    glGenVertexArrays(1, &cubeVAO);
    glGenBuffers(1, &cubeVBO);
    glBindVertexArray(cubeVAO);
    glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
    glBufferData(GL_ARRAY_BUFFER, verts.size() * sizeof(float), verts.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 9*sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 9*sizeof(float), (void*)(3*sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 9*sizeof(float), (void*)(6*sizeof(float)));
    glEnableVertexAttribArray(2);
    glBindVertexArray(0);
}

void Renderer::buildPyramidMesh() {
    std::vector<float> verts;
    // Base quad
    float r = 0.2f, g = 0.8f, b = 0.6f; // Emerald green
    float s = 0.5f, h = 1.0f;
    // 4 triangular faces
    // Front
    pushVertex(verts, 0,h,0, r,g,b, 0,0.5f,1); pushVertex(verts, -s,0,s, r,g,b, 0,0.5f,1); pushVertex(verts, s,0,s, r,g,b, 0,0.5f,1);
    // Right
    pushVertex(verts, 0,h,0, r,g,b, 1,0.5f,0); pushVertex(verts, s,0,s, r,g,b, 1,0.5f,0); pushVertex(verts, s,0,-s, r,g,b, 1,0.5f,0);
    // Back
    pushVertex(verts, 0,h,0, r,g,b, 0,0.5f,-1); pushVertex(verts, s,0,-s, r,g,b, 0,0.5f,-1); pushVertex(verts, -s,0,-s, r,g,b, 0,0.5f,-1);
    // Left
    pushVertex(verts, 0,h,0, r,g,b, -1,0.5f,0); pushVertex(verts, -s,0,-s, r,g,b, -1,0.5f,0); pushVertex(verts, -s,0,s, r,g,b, -1,0.5f,0);
    // Base
    pushVertex(verts, -s,0,s, r,g,b, 0,-1,0); pushVertex(verts, s,0,s, r,g,b, 0,-1,0); pushVertex(verts, s,0,-s, r,g,b, 0,-1,0);
    pushVertex(verts, -s,0,s, r,g,b, 0,-1,0); pushVertex(verts, s,0,-s, r,g,b, 0,-1,0); pushVertex(verts, -s,0,-s, r,g,b, 0,-1,0);

    pyramidVertexCount = (int)(verts.size() / 9);

    glGenVertexArrays(1, &pyramidVAO);
    glGenBuffers(1, &pyramidVBO);
    glBindVertexArray(pyramidVAO);
    glBindBuffer(GL_ARRAY_BUFFER, pyramidVBO);
    glBufferData(GL_ARRAY_BUFFER, verts.size() * sizeof(float), verts.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 9*sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 9*sizeof(float), (void*)(3*sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 9*sizeof(float), (void*)(6*sizeof(float)));
    glEnableVertexAttribArray(2);
    glBindVertexArray(0);
}

void Renderer::buildSphereMesh() {
    std::vector<float> verts;
    float r = 0.6f, g = 0.2f, b = 0.9f; // Ender pearl purple
    int stacks = 8, slices = 12;
    float radius = 0.4f;

    for (int i = 0; i < stacks; i++) {
        float phi0 = glm::pi<float>() * (float)i / stacks - glm::half_pi<float>();
        float phi1 = glm::pi<float>() * (float)(i + 1) / stacks - glm::half_pi<float>();
        for (int j = 0; j < slices; j++) {
            float theta0 = 2.0f * glm::pi<float>() * (float)j / slices;
            float theta1 = 2.0f * glm::pi<float>() * (float)(j + 1) / slices;

            auto spherePos = [&](float phi, float theta) -> glm::vec3 {
                return glm::vec3(
                    radius * cos(phi) * cos(theta),
                    radius * sin(phi),
                    radius * cos(phi) * sin(theta)
                );
            };

            glm::vec3 p0 = spherePos(phi0, theta0);
            glm::vec3 p1 = spherePos(phi0, theta1);
            glm::vec3 p2 = spherePos(phi1, theta1);
            glm::vec3 p3 = spherePos(phi1, theta0);

            glm::vec3 n0 = glm::normalize(p0);
            glm::vec3 n1 = glm::normalize(p1);
            glm::vec3 n2 = glm::normalize(p2);
            glm::vec3 n3 = glm::normalize(p3);

            pushVertex(verts, p0.x,p0.y,p0.z, r,g,b, n0.x,n0.y,n0.z);
            pushVertex(verts, p1.x,p1.y,p1.z, r,g,b, n1.x,n1.y,n1.z);
            pushVertex(verts, p2.x,p2.y,p2.z, r,g,b, n2.x,n2.y,n2.z);

            pushVertex(verts, p0.x,p0.y,p0.z, r,g,b, n0.x,n0.y,n0.z);
            pushVertex(verts, p2.x,p2.y,p2.z, r,g,b, n2.x,n2.y,n2.z);
            pushVertex(verts, p3.x,p3.y,p3.z, r,g,b, n3.x,n3.y,n3.z);
        }
    }

    sphereVertexCount = (int)(verts.size() / 9);

    glGenVertexArrays(1, &sphereVAO);
    glGenBuffers(1, &sphereVBO);
    glBindVertexArray(sphereVAO);
    glBindBuffer(GL_ARRAY_BUFFER, sphereVBO);
    glBufferData(GL_ARRAY_BUFFER, verts.size() * sizeof(float), verts.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 9*sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 9*sizeof(float), (void*)(3*sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 9*sizeof(float), (void*)(6*sizeof(float)));
    glEnableVertexAttribArray(2);
    glBindVertexArray(0);
}

void Renderer::renderCollectibles(Shader& shader, const glm::mat4& view,
                                  const glm::mat4& projection,
                                  const std::vector<CollectibleItem>& items,
                                  const glm::vec3& sunDir, const glm::vec3& sunColor,
                                  float ambientLevel, const glm::vec3& fogCol,
                                  bool torchEnabled, const glm::vec3& torchPos,
                                  const glm::vec3& torchColor, float torchRadius) {
    shader.use();
    shader.setMat4("view", view);
    shader.setMat4("projection", projection);
    setLightingUniforms(shader, sunDir, sunColor, ambientLevel, fogCol,
                        torchEnabled, torchPos, torchColor, torchRadius);

    // All collectables use the gift box texture and rectangular mesh
    shader.setBool("useTexture", true);
    shader.setBool("enableEdgeOutline", false);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, giftBoxTextureID);
    shader.setInt("wallTexture", 0);

    for (auto& item : items) {
        // Skip collected or picked-up items (no longer in the world)
        if (item.collected || item.pickedUp) continue;

        glm::mat4 model = glm::translate(glm::mat4(1.0f), item.position);
        model = glm::rotate(model, glm::radians(item.rotationAngle), glm::vec3(0, 1, 0));

        float bob = sin(item.rotationAngle * 0.03f) * 0.15f;
        model = glm::translate(model, glm::vec3(0, bob, 0));
        model = glm::scale(model, glm::vec3(COLLECTIBLE_SIZE));

        shader.setMat4("model", model);

        // Render as textured gift box (axis-aligned rectangle/box)
        glBindVertexArray(giftBoxVAO);
        glDrawArrays(GL_TRIANGLES, 0, giftBoxVertexCount);
        glBindVertexArray(0);
    }
}

void Renderer::renderCarriedCollectible(Shader& shader, const glm::mat4& view,
                                        const glm::mat4& projection,
                                        const glm::vec3& playerPos,
                                        const glm::vec3& playerFront,
                                        int carryCount,
                                        const glm::vec3& sunDir, const glm::vec3& sunColor,
                                        float ambientLevel, const glm::vec3& fogCol,
                                        bool torchEnabled, const glm::vec3& torchPos,
                                        const glm::vec3& torchColor, float torchRadius) {
    if (carryCount <= 0) return;

    shader.use();
    shader.setMat4("view", view);
    shader.setMat4("projection", projection);
    setLightingUniforms(shader, sunDir, sunColor, ambientLevel, fogCol,
                        torchEnabled, torchPos, torchColor, torchRadius);

    // Use gift box texture
    shader.setBool("useTexture", true);
    shader.setBool("enableEdgeOutline", false);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, giftBoxTextureID);
    shader.setInt("wallTexture", 0);

    // Base position: slightly in front, to the right, below eye level
    glm::vec3 flatFront = glm::normalize(glm::vec3(playerFront.x, 0.0f, playerFront.z));
    glm::vec3 right = glm::normalize(glm::cross(flatFront, glm::vec3(0.0f, 1.0f, 0.0f)));
    glm::vec3 basePos = playerPos
                      + flatFront * CARRY_FORWARD_OFFSET
                      + right * CARRY_RIGHT_OFFSET
                      + glm::vec3(0.0f, CARRY_Y_OFFSET, 0.0f);

    // Stack boxes vertically; each box is COLLECTIBLE_SIZE above the previous
    for (int i = 0; i < carryCount; i++) {
        glm::vec3 stackPos = basePos + glm::vec3(0.0f, i * COLLECTIBLE_SIZE, 0.0f);
        glm::mat4 model = glm::translate(glm::mat4(1.0f), stackPos);
        model = glm::scale(model, glm::vec3(COLLECTIBLE_SIZE));
        shader.setMat4("model", model);

        glBindVertexArray(giftBoxVAO);
        glDrawArrays(GL_TRIANGLES, 0, giftBoxVertexCount);
        glBindVertexArray(0);
    }
}

void Renderer::renderExitZone(Shader& shader, const glm::mat4& view,
                              const glm::mat4& projection,
                              const glm::vec3& exitPos, float time,
                              const glm::vec3& sunDir, const glm::vec3& sunColor,
                              float ambientLevel, const glm::vec3& fogCol,
                              bool torchEnabled, const glm::vec3& torchPos,
                              const glm::vec3& torchColor, float torchRadius) {
    shader.use();
    shader.setMat4("view", view);
    shader.setMat4("projection", projection);
    setLightingUniforms(shader, sunDir, sunColor, ambientLevel, fogCol,
                        torchEnabled, torchPos, torchColor, torchRadius);
    shader.setBool("useTexture", false);
    shader.setBool("enableEdgeOutline", false);

    // Helper: render a cube at given world position and scale with given color
    auto drawCube = [&](glm::vec3 pos, glm::vec3 scale, float r, float g, float b) {
        std::vector<float> v;
        addCube(v, -0.5f, -0.5f, -0.5f, scale.x, scale.y, scale.z, r, g, b);
        glBindVertexArray(scratchVAO);
        glBindBuffer(GL_ARRAY_BUFFER, scratchVBO);
        glBufferData(GL_ARRAY_BUFFER, v.size() * sizeof(float), v.data(), GL_DYNAMIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 9*sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 9*sizeof(float), (void*)(3*sizeof(float)));
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 9*sizeof(float), (void*)(6*sizeof(float)));
        glEnableVertexAttribArray(2);

        glm::mat4 model = glm::translate(glm::mat4(1.0f), pos);
        shader.setMat4("model", model);
        glDrawArrays(GL_TRIANGLES, 0, (int)(v.size() / 9));
        glBindVertexArray(0);
    };

    // --- Exit Zone: Gateway to Infinite Outside World ---

    // Large platform representing the transition area
    float platformSize = 6.0f;  // Larger platform (6x6 units = 3 cells)
    drawCube(exitPos + glm::vec3(0.0f, 0.05f, 0.0f),
             glm::vec3(platformSize, 0.1f, platformSize),
             0.8f, 0.85f, 0.9f);  // Light, almost white stone

    // Create a visual boundary arch/gateway
    // Two large pillars on either side
    float archHeight = 5.0f;
    float archWidth = 0.4f;
    float archSpacing = 3.5f;

    // Left pillar
    drawCube(exitPos + glm::vec3(-archSpacing, archHeight * 0.5f, 0.0f),
             glm::vec3(archWidth, archHeight, archWidth),
             0.6f, 0.65f, 0.7f);

    // Right pillar
    drawCube(exitPos + glm::vec3(archSpacing, archHeight * 0.5f, 0.0f),
             glm::vec3(archWidth, archHeight, archWidth),
             0.6f, 0.65f, 0.7f);

    // Archway top (lintel)
    drawCube(exitPos + glm::vec3(0.0f, archHeight, 0.0f),
             glm::vec3(archSpacing * 2.0f + archWidth, archWidth, archWidth),
             0.6f, 0.65f, 0.7f);

    // Glowing energy curtain representing the exit portal
    float curtainPulse = 0.3f + 0.2f * sin(time * 1.5f);
    float curtainAlpha = 0.4f + 0.3f * sin(time * 2.0f);
    drawCube(exitPos + glm::vec3(0.0f, archHeight * 0.5f, 0.0f),
             glm::vec3(archSpacing * 2.0f - archWidth, archHeight * 0.9f, 0.1f),
             0.5f * curtainPulse, 0.8f * curtainPulse, 1.0f * curtainPulse);

    // Four corner markers showing the boundary of the exit zone
    float markerH = 1.5f;
    float markerW = 0.2f;
    float markerOff = platformSize * 0.45f;
    for (float px : {-markerOff, markerOff}) {
        for (float pz : {-markerOff, markerOff}) {
            // Marker pillar
            drawCube(exitPos + glm::vec3(px, markerH * 0.5f, pz),
                     glm::vec3(markerW, markerH, markerW),
                     0.5f, 0.55f, 0.6f);

            // Pulsing light at top
            float glowPulse = 0.5f + 0.5f * sin(time * 2.0f + px + pz);
            drawCube(exitPos + glm::vec3(px, markerH + 0.15f, pz),
                     glm::vec3(markerW * 2.0f, 0.15f, markerW * 2.0f),
                     0.3f * glowPulse, 0.9f * glowPulse, 1.0f * glowPulse);
        }
    }

    // Central floating beacon
    float floatHeight = 1.5f + sin(time * 1.5f) * 0.2f;
    float centralGlow = 0.6f + 0.4f * sin(time * 2.5f);
    drawCube(exitPos + glm::vec3(0.0f, floatHeight, 0.0f),
             glm::vec3(0.4f, 0.4f, 0.4f),
             0.4f * centralGlow, 0.9f * centralGlow, 1.0f * centralGlow);

    // Orbiting particles around the beacon (representing freedom/escape)
    for (int i = 0; i < 6; i++) {
        float angle = time * 2.5f + i * (6.283f / 6.0f); // 60° apart
        float radius = 0.7f;
        float particleY = floatHeight + sin(time * 3.0f + i * 1.2f) * 0.15f;
        glm::vec3 ppos = exitPos + glm::vec3(
            radius * cos(angle), particleY, radius * sin(angle));
        float intensity = 0.5f + 0.5f * sin(time * 4.0f + i);
        drawCube(ppos, glm::vec3(0.1f),
                 0.3f * intensity, 0.8f * intensity, 1.0f * intensity);
    }

    // Add ground-level light rings expanding outward (representing the infinite world)
    for (int ring = 0; ring < 3; ring++) {
        float ringRadius = 2.5f + ring * 1.5f;
        float ringPulse = sin(time * 2.0f - ring * 0.5f) * 0.5f + 0.5f;
        int segments = 12;
        for (int seg = 0; seg < segments; seg++) {
            float angle = (seg / (float)segments) * 6.283f;
            glm::vec3 ringPos = exitPos + glm::vec3(
                ringRadius * cos(angle), 0.2f, ringRadius * sin(angle));
            drawCube(ringPos, glm::vec3(0.15f, 0.1f, 0.15f),
                     0.4f * ringPulse, 0.7f * ringPulse, 0.9f * ringPulse);
        }
    }
}


void Renderer::renderPlayer(Shader& shader, const glm::mat4& view,
                            const glm::mat4& projection,
                            const glm::vec3& playerPos, float yaw,
                            const glm::vec3& sunDir, const glm::vec3& sunColor,
                            float ambientLevel, const glm::vec3& fogCol,
                            bool torchEnabled, const glm::vec3& torchPos,
                            const glm::vec3& torchColor, float torchRadius) {
    shader.use();
    shader.setMat4("view", view);
    shader.setMat4("projection", projection);
    setLightingUniforms(shader, sunDir, sunColor, ambientLevel, fogCol,
                        torchEnabled, torchPos, torchColor, torchRadius);
    shader.setBool("useTexture", false);
    shader.setBool("enableEdgeOutline", false);

    // playerPos is at eye height (1.7 units above ground)
    // foot level = playerPos.y - cameraHeight (1.7)
    float footY = playerPos.y - 1.7f;

    // Helper: draw a box at world position with given half-extents and color
    auto drawBox = [&](glm::vec3 center, glm::vec3 scale, float r, float g, float b) {
        std::vector<float> v;
        addCube(v, -0.5f, -0.5f, -0.5f, scale.x, scale.y, scale.z, r, g, b);
        glBindVertexArray(playerScratchVAO);
        glBindBuffer(GL_ARRAY_BUFFER, playerScratchVBO);
        glBufferData(GL_ARRAY_BUFFER, v.size() * sizeof(float), v.data(), GL_DYNAMIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 9*sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 9*sizeof(float), (void*)(3*sizeof(float)));
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 9*sizeof(float), (void*)(6*sizeof(float)));
        glEnableVertexAttribArray(2);

        glm::mat4 model = glm::translate(glm::mat4(1.0f), center);
        model = glm::rotate(model, glm::radians(yaw + 90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        shader.setMat4("model", model);
        glDrawArrays(GL_TRIANGLES, 0, (int)(v.size() / 9));
        glBindVertexArray(0);
    };

    // Legs (two blocks)
    float legW = 0.22f, legH = 0.55f, legD = 0.22f;
    drawBox(glm::vec3(playerPos.x, footY + legH * 0.5f, playerPos.z) + glm::vec3( 0.12f, 0.0f, 0.0f),
            glm::vec3(legW, legH, legD), 0.20f, 0.25f, 0.65f); // left leg (blue)
    drawBox(glm::vec3(playerPos.x, footY + legH * 0.5f, playerPos.z) + glm::vec3(-0.12f, 0.0f, 0.0f),
            glm::vec3(legW, legH, legD), 0.20f, 0.25f, 0.65f); // right leg

    // Body (torso)
    float bodyW = 0.50f, bodyH = 0.65f, bodyD = 0.28f;
    float bodyY = footY + legH + bodyH * 0.5f;
    drawBox(glm::vec3(playerPos.x, bodyY, playerPos.z),
            glm::vec3(bodyW, bodyH, bodyD), 0.20f, 0.55f, 0.25f); // green shirt

    // Arms
    float armW = 0.18f, armH = 0.58f, armD = 0.18f;
    float armY = footY + legH + armH * 0.5f + 0.05f;
    float armOff = (bodyW + armW) * 0.5f;
    drawBox(glm::vec3(playerPos.x, armY, playerPos.z) + glm::vec3( armOff, 0.0f, 0.0f),
            glm::vec3(armW, armH, armD), 0.80f, 0.60f, 0.45f); // skin
    drawBox(glm::vec3(playerPos.x, armY, playerPos.z) + glm::vec3(-armOff, 0.0f, 0.0f),
            glm::vec3(armW, armH, armD), 0.80f, 0.60f, 0.45f);

    // Head (rectangular, wider than body)
    float headW = 0.58f, headH = 0.45f, headD = 0.48f;
    float headY = footY + legH + bodyH + headH * 0.5f;
    drawBox(glm::vec3(playerPos.x, headY, playerPos.z),
            glm::vec3(headW, headH, headD), 0.80f, 0.60f, 0.45f); // skin tone

    // Eyes (two small dark blocks on front face)
    float eyeW = 0.08f, eyeH = 0.07f, eyeD = 0.05f;
    float eyeY = headY + 0.05f;
    float eyeOff = 0.13f;
    drawBox(glm::vec3(playerPos.x, eyeY, playerPos.z) + glm::vec3( eyeOff, 0.0f, headD * 0.5f),
            glm::vec3(eyeW, eyeH, eyeD), 0.10f, 0.10f, 0.10f);
    drawBox(glm::vec3(playerPos.x, eyeY, playerPos.z) + glm::vec3(-eyeOff, 0.0f, headD * 0.5f),
            glm::vec3(eyeW, eyeH, eyeD), 0.10f, 0.10f, 0.10f);
}
