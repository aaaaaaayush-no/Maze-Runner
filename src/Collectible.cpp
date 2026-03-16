#include "Collectible.h"
#include <cmath>

static const float CELL_SIZE = 2.0f;

Collectible::Collectible() {}

void Collectible::placeItems(const std::vector<std::pair<int,int>>& positions) {
    items.clear();
    ItemType types[] = {ItemType::KEY, ItemType::ARTIFACT, ItemType::ORB};
    int i = 0;
    for (auto [gx, gy] : positions) {
        CollectibleItem item;
        item.position = glm::vec3(
            gx * CELL_SIZE + CELL_SIZE * 0.5f,
            1.0f, // floating height
            gy * CELL_SIZE + CELL_SIZE * 0.5f
        );
        item.size = glm::vec2(COLLECTIBLE_SIZE, COLLECTIBLE_SIZE);
        item.type = types[i % 3];
        item.collected = false;
        item.pickedUp = false;
        item.rotationAngle = 0.0f;
        items.push_back(item);
        i++;
    }
}

void Collectible::update(float dt) {
    for (auto& item : items) {
        // Only rotate items still in the world (not collected/picked up)
        if (!item.collected && !item.pickedUp)
            item.rotationAngle += 90.0f * dt; // degrees per second
    }
}

bool Collectible::tryCollect(const glm::vec3& playerPos, float radius) {
    bool collected = false;
    for (auto& item : items) {
        if (item.collected) continue;
        float dist = glm::length(playerPos - item.position);
        if (dist < radius) {
            item.collected = true;
            collected = true;
        }
    }
    return collected;
}

// AABB overlap pickup: returns index of first overlapping uncollected item, or -1
int Collectible::tryPickup(const glm::vec3& playerPos, bool /*alreadyCarrying*/) {
    // Always allow pickup regardless of carry state (stackable items)
    for (int i = 0; i < (int)items.size(); i++) {
        auto& item = items[i];
        if (item.collected || item.pickedUp) continue;

        // AABB overlap test between player and collectable
        float dx = std::abs(playerPos.x - item.position.x);
        float dy = std::abs(playerPos.y - item.position.y);
        float dz = std::abs(playerPos.z - item.position.z);

        if (dx < PICKUP_HALF_WIDTH &&
            dy < PICKUP_HALF_HEIGHT &&
            dz < PICKUP_HALF_WIDTH) {
            // Mark as picked up (carried), but NOT collected yet.
            // Items become "collected" only when deposited at the cauldron.
            item.pickedUp = true;
            return i;
        }
    }
    return -1;
}

int Collectible::getCollectedCount() const {
    // Counts items that have been grabbed (carried) OR deposited at the cauldron,
    // useful for the HUD progress counter.
    int count = 0;
    for (auto& item : items)
        if (item.collected || item.pickedUp) count++;
    return count;
}

int Collectible::getDepositedCount() const {
    int count = 0;
    for (auto& item : items)
        if (item.collected) count++;
    return count;
}

int Collectible::getTotalCount() const {
    return (int)items.size();
}

bool Collectible::allCollected() const {
    // All items must be deposited (collected=true) for win condition
    if (getTotalCount() == 0) return false;
    for (auto& item : items)
        if (!item.collected) return false;
    return true;
}
