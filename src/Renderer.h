#ifndef RENDERER_H
#define RENDERER_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <vector>
#include <string>

class Maze;
class Shader;
struct CollectibleItem;

class Renderer {
public:
    Renderer();
    ~Renderer();

    void init();
    void buildMazeMesh(const Maze& maze);
    void renderMaze(Shader& shader, const glm::mat4& view, const glm::mat4& projection,
                    const glm::vec3& sunDir, const glm::vec3& sunColor,
                    float ambientLevel, const glm::vec3& fogCol,
                    bool torchEnabled, const glm::vec3& torchPos,
                    const glm::vec3& torchColor, float torchRadius);
    void renderCollectibles(Shader& shader, const glm::mat4& view,
                            const glm::mat4& projection,
                            const std::vector<CollectibleItem>& items,
                            const glm::vec3& sunDir, const glm::vec3& sunColor,
                            float ambientLevel, const glm::vec3& fogCol,
                            bool torchEnabled, const glm::vec3& torchPos,
                            const glm::vec3& torchColor, float torchRadius);
    void renderExitPortal(Shader& shader, const glm::mat4& view,
                          const glm::mat4& projection,
                          const glm::vec3& exitPos, float time,
                          const glm::vec3& sunDir, const glm::vec3& sunColor,
                          float ambientLevel, const glm::vec3& fogCol,
                          bool torchEnabled, const glm::vec3& torchPos,
                          const glm::vec3& torchColor, float torchRadius);

    void setWireframe(bool enabled);

private:
    // Maze geometry
    unsigned int mazeVAO, mazeVBO;
    int mazeVertexCount;

    // Cube geometry (for keys)
    unsigned int cubeVAO, cubeVBO;
    int cubeVertexCount;

    // Pyramid geometry (for artifacts)
    unsigned int pyramidVAO, pyramidVBO;
    int pyramidVertexCount;

    // Sphere geometry (for orbs)
    unsigned int sphereVAO, sphereVBO;
    int sphereVertexCount;

    // Wall texture
    unsigned int wallTextureID;

    // Graffiti system
    std::vector<unsigned int> graffitiTextureIDs;
    unsigned int graffitiVAO, graffitiVBO;
    int graffitiVertexCount;

    // Batch rendering info for graffiti (rebuilt per buildMazeMesh)
    struct GraffitiBatchInfo {
        int startVertex;
        int vertexCount;
        int textureIdx;
    };
    std::vector<GraffitiBatchInfo> graffitiBatchInfos_;

    // Graffiti config: filenames to load from textures/graffiti/
    static const std::vector<std::string> GRAFFITI_FILES;
    static constexpr float GRAFFITI_CHANCE = 0.05f; // 5% chance per eligible wall face

    bool wireframe;

    void buildCubeMesh();
    void buildPyramidMesh();
    void buildSphereMesh();
    void generateWallTexture();
    void generateGraffitiTextures();
    void buildGraffitiMesh(const Maze& maze);

    void addCube(std::vector<float>& verts,
                 float x, float y, float z,
                 float sx, float sy, float sz,
                 float r, float g, float b);
};

#endif
