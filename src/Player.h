#ifndef PLAYER_H
#define PLAYER_H

#include <glm/glm.hpp>

class Maze;

class Player {
public:
    glm::vec3 position;
    float yaw;
    float pitch;

    Player();

    void init(float startX, float startZ);
    void processMouseMovement(float xOffset, float yOffset);
    void update(float dt, const Maze& maze, bool moveForward, bool moveBack,
                bool moveLeft, bool moveRight, bool jump);

    glm::mat4 getViewMatrix() const;
    glm::vec3 getFront() const;

    bool isOnGround() const { return onGround; }
    int getCollectedCount() const { return 0; } // Managed externally

private:
    float moveSpeed;
    float mouseSensitivity;
    float cameraHeight;

    // Physics
    float velocityY;
    float gravity;
    float jumpForce;
    bool onGround;

    bool checkCollision(const glm::vec3& pos, const Maze& maze) const;
};

#endif
