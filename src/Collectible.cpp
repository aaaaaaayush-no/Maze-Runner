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
        item.type = types[i % 3];
        item.collected = false;
        item.rotationAngle = 0.0f;
        items.push_back(item);
        i++;
    }
}

void Collectible::update(float dt) {
    for (auto& item : items) {
        if (!item.collected)
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

int Collectible::getCollectedCount() const {
    int count = 0;
    for (auto& item : items)
        if (item.collected) count++;
    return count;
}

int Collectible::getTotalCount() const {
    return (int)items.size();
}

bool Collectible::allCollected() const {
    return getCollectedCount() == getTotalCount() && getTotalCount() > 0;
}
