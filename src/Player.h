#ifndef PLAYER_H
#define PLAYER_H

#include <glm/glm.hpp>
#include <vector>

class Maze;

class Player {
public:
    glm::vec3 position;
    float yaw;
    float pitch;

    // Stack of carried collectible indices (into Collectible::items)
    std::vector<int> carriedItems;

    // Third-person perspective toggle
    bool thirdPerson;

    Player();

    void init(float startX, float startZ);
    void processMouseMovement(float xOffset, float yOffset);
    void update(float dt, const Maze& maze, bool moveForward, bool moveBack,
                bool moveLeft, bool moveRight, bool jump);

    glm::mat4 getViewMatrix() const;
    glm::vec3 getFront() const;
    glm::vec3 getCameraPosition() const;

    bool isOnGround() const { return onGround; }

private:
    float moveSpeed;
    float mouseSensitivity;
    float cameraHeight;

    // Physics
    float velocityY;
    float gravity;
    float jumpForce;
    bool onGround;

    // Third-person camera state
    mutable glm::vec3 currentCameraPos;
    mutable bool cameraInitialized;
    float cameraDistance;
    float cameraHeight3P;
    float cameraFollowSpeed;
    float cameraMinDistance;
    float cameraMaxDistance;

    bool checkCollision(const glm::vec3& pos, const Maze& maze) const;
    glm::vec3 calculateDesiredCameraPosition() const;
    bool checkCameraObstruction(const glm::vec3& from, const glm::vec3& to, const Maze& maze) const;
};

#endif
