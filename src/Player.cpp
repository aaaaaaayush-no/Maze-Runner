#include "Player.h"
#include "Maze.h"
#include <glm/gtc/matrix_transform.hpp>
#include <cmath>

// Each maze cell maps to 2.0 world units (corridor width)
static const float CELL_SIZE = 2.0f;

Player::Player()
    : position(0.0f)
    , yaw(-90.0f)
    , pitch(0.0f)
    , moveSpeed(5.0f)
    , mouseSensitivity(0.1f)
    , cameraHeight(1.7f)
    , velocityY(0.0f)
    , gravity(9.8f)
    , jumpForce(4.5f)
    , onGround(true)
{
}

void Player::init(float startX, float startZ) {
    position = glm::vec3(startX * CELL_SIZE + CELL_SIZE * 0.5f,
                         cameraHeight,
                         startZ * CELL_SIZE + CELL_SIZE * 0.5f);
    yaw = -90.0f;
    pitch = 0.0f;
    velocityY = 0.0f;
    onGround = true;
}

void Player::processMouseMovement(float xOffset, float yOffset) {
    xOffset *= mouseSensitivity;
    yOffset *= mouseSensitivity;

    yaw   += xOffset;
    pitch += yOffset;

    if (pitch >  89.0f) pitch =  89.0f;
    if (pitch < -89.0f) pitch = -89.0f;
}

glm::vec3 Player::getFront() const {
    glm::vec3 front;
    front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    front.y = sin(glm::radians(pitch));
    front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    return glm::normalize(front);
}

glm::mat4 Player::getViewMatrix() const {
    glm::vec3 front = getFront();
    return glm::lookAt(position, position + front, glm::vec3(0.0f, 1.0f, 0.0f));
}

bool Player::checkCollision(const glm::vec3& pos, const Maze& maze) const {
    // Player bounding box: 0.5 x height x 0.5 (half-width = 0.25)
    float hw = 0.25f;
    // Check four corners at foot level
    for (float dx : {-hw, hw}) {
        for (float dz : {-hw, hw}) {
            float wx = pos.x + dx;
            float wz = pos.z + dz;
            int gx = (int)std::floor(wx / CELL_SIZE);
            int gz = (int)std::floor(wz / CELL_SIZE);
            if (maze.isWall(gx, gz))
                return true;
        }
    }
    return false;
}

void Player::update(float dt, const Maze& maze, bool moveForward, bool moveBack,
                    bool moveLeft, bool moveRight, bool jump) {
    // Horizontal movement
    glm::vec3 front = getFront();
    glm::vec3 flatFront = glm::normalize(glm::vec3(front.x, 0.0f, front.z));
    glm::vec3 right = glm::normalize(glm::cross(flatFront, glm::vec3(0.0f, 1.0f, 0.0f)));

    glm::vec3 moveDir(0.0f);
    if (moveForward) moveDir += flatFront;
    if (moveBack)    moveDir -= flatFront;
    if (moveRight)   moveDir += right;
    if (moveLeft)    moveDir -= right;

    if (glm::length(moveDir) > 0.001f)
        moveDir = glm::normalize(moveDir);

    glm::vec3 horizontalMove = moveDir * moveSpeed * dt;

    // Try X movement
    glm::vec3 newPos = position;
    newPos.x += horizontalMove.x;
    if (!checkCollision(newPos, maze))
        position.x = newPos.x;

    // Try Z movement
    newPos = position;
    newPos.z += horizontalMove.z;
    if (!checkCollision(newPos, maze))
        position.z = newPos.z;

    // Jump
    if (jump && onGround) {
        velocityY = jumpForce;
        onGround = false;
    }

    // Gravity
    velocityY -= gravity * dt;
    position.y += velocityY * dt;

    // Ground collision
    if (position.y <= cameraHeight) {
        position.y = cameraHeight;
        velocityY = 0.0f;
        onGround = true;
    }
}
