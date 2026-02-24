#ifndef MINIMAP_H
#define MINIMAP_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <vector>
#include <set>

class Maze;
class Shader;
struct CollectibleItem;

class Minimap {
public:
    Minimap();
    ~Minimap();

    void init();
    void setVisible(bool vis) { visible = vis; }
    bool isVisible() const { return visible; }
    void toggleVisible() { visible = !visible; }

    void toggleLegend() { legendVisible = !legendVisible; }
    bool isLegendVisible() const { return legendVisible; }

    void markExplored(int gx, int gy);
    void clearExplored();

    // dt for smooth viewport lerp
    void render(Shader& shader, const Maze& maze,
                const glm::vec3& playerPos, float playerYaw,
                const std::vector<CollectibleItem>& items,
                const glm::vec3& exitPos,
                int screenWidth, int screenHeight,
                int difficulty = 1, float dt = 0.016f);

private:
    bool visible;
    bool legendVisible;
    unsigned int vao, vbo;
    std::set<std::pair<int,int>> explored;

    // Smooth viewport camera position for scrolling
    float viewCamX, viewCamY;

    void pushQuad2D(std::vector<float>& v,
                    float x0, float y0, float x1, float y1,
                    float r, float g, float b);
    void pushTriangle2D(std::vector<float>& v,
                        float x0, float y0,
                        float x1, float y1,
                        float x2, float y2,
                        float r, float g, float b);
};

#endif
