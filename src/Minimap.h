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

    void markExplored(int gx, int gy);
    void clearExplored();

    void render(Shader& shader, const Maze& maze,
                const glm::vec3& playerPos, float playerYaw,
                const std::vector<CollectibleItem>& items,
                const glm::vec3& exitPos,
                int screenWidth, int screenHeight);

private:
    bool visible;
    unsigned int vao, vbo;
    std::set<std::pair<int,int>> explored;

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
