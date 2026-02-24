#ifndef COLLECTIBLE_H
#define COLLECTIBLE_H

#include <glm/glm.hpp>
#include <vector>

enum class ItemType {
    KEY,       // Golden cube
    ARTIFACT,  // Rotating pyramid
    ORB        // Sphere (rendered as low-poly icosphere)
};

struct CollectibleItem {
    glm::vec3 position;
    ItemType type;
    bool collected;
    float rotationAngle;
};

class Collectible {
public:
    Collectible();

    void placeItems(const std::vector<std::pair<int,int>>& positions);
    void update(float dt);
    bool tryCollect(const glm::vec3& playerPos, float radius = 1.5f);

    int getCollectedCount() const;
    int getTotalCount() const;
    bool allCollected() const;

    const std::vector<CollectibleItem>& getItems() const { return items; }

private:
    std::vector<CollectibleItem> items;
};

#endif
