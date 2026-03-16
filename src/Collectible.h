#ifndef COLLECTIBLE_H
#define COLLECTIBLE_H

#include <glm/glm.hpp>
#include <vector>

// Collectable box dimensions in world units
static constexpr float COLLECTIBLE_SIZE      = 0.3f;
static constexpr float COLLECTIBLE_HALF_SIZE = COLLECTIBLE_SIZE * 0.5f;

// AABB pickup range (generous, similar feel to old sphere radius 1.5)
static constexpr float PICKUP_HALF_WIDTH  = 0.75f;  // XZ half-extent for pickup
static constexpr float PICKUP_HALF_HEIGHT = 1.25f;  // Y half-extent for pickup

// Carry behavior constants (CARRY_FORWARD_OFFSET, Y_OFFSET, RIGHT_OFFSET define hand position)
static constexpr float CARRY_FORWARD_OFFSET  = 0.8f;   // Units in front of player
static constexpr float CARRY_Y_OFFSET        = -0.6f;   // Units below player eye level
static constexpr float CARRY_RIGHT_OFFSET    = 0.3f;   // Units to the right of player

enum class ItemType {
    KEY,       // Golden cube
    ARTIFACT,  // Rotating pyramid
    ORB        // Sphere (rendered as low-poly icosphere)
};

struct CollectibleItem {
    glm::vec3 position;
    glm::vec2 size;          // Rectangular size (width, height) in world units
    ItemType type;
    bool collected;
    bool pickedUp;           // Whether the item is currently carried by the player
    float rotationAngle;
};

class Collectible {
public:
    Collectible();

    void placeItems(const std::vector<std::pair<int,int>>& positions);
    void update(float dt);
    bool tryCollect(const glm::vec3& playerPos, float radius = 1.5f);

    // AABB-based pickup: returns index of picked-up item, or -1 if none
    int tryPickup(const glm::vec3& playerPos, bool alreadyCarrying);

    // Count of items grabbed (pickedUp OR deposited) – used for HUD progress display
    int getCollectedCount() const;
    // Count of items actually deposited at the cauldron (collected=true)
    int getDepositedCount() const;
    int getTotalCount() const;
    bool allCollected() const;

    const std::vector<CollectibleItem>& getItems() const { return items; }
    std::vector<CollectibleItem>& getItemsMut() { return items; }

private:
    std::vector<CollectibleItem> items;
};

#endif
