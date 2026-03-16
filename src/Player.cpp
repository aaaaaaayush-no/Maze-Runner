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
    , thirdPerson(false)
    , moveSpeed(5.0f)
    , mouseSensitivity(0.1f)
    , cameraHeight(1.7f)
    , velocityY(0.0f)
    , gravity(9.8f)
    , jumpForce(4.5f)
    , onGround(true)
    , currentCameraPos(0.0f)
    , cameraInitialized(false)
    , cameraDistance(4.0f)
    , cameraHeight3P(2.5f)
    , cameraFollowSpeed(8.0f)
    , cameraMinDistance(2.0f)
    , cameraMaxDistance(6.0f)
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
    carriedItems.clear();
    cameraInitialized = false;
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

glm::vec3 Player::calculateDesiredCameraPosition() const {
    glm::vec3 front = getFront();
    // Use flat front (no pitch) for camera positioning
    glm::vec3 flatFront = glm::normalize(glm::vec3(front.x, 0.0f, front.z));

    // Calculate position behind and above the player
    glm::vec3 desiredPos = position - flatFront * cameraDistance;
    desiredPos.y = position.y + cameraHeight3P;

    return desiredPos;
}

bool Player::checkCameraObstruction(const glm::vec3& from, const glm::vec3& to, const Maze& maze) const {
    // Raycast from player to camera position to check for walls
    glm::vec3 dir = to - from;
    float distance = glm::length(dir);
    if (distance < 0.001f) return false;

    dir = glm::normalize(dir);

    // Sample along the ray
    int steps = (int)(distance / 0.2f) + 1;
    for (int i = 1; i <= steps; i++) {
        float t = (float)i / (float)steps;
        glm::vec3 samplePos = from + dir * (distance * t);

        int gx = (int)std::floor(samplePos.x / CELL_SIZE);
        int gz = (int)std::floor(samplePos.z / CELL_SIZE);

        if (maze.isWall(gx, gz)) {
            return true;
        }
    }
    return false;
}

glm::vec3 Player::getCameraPosition() const {
    if (thirdPerson) {
        return currentCameraPos;
    }
    return position;
}

glm::mat4 Player::getViewMatrix() const {
    glm::vec3 front = getFront();

    if (thirdPerson) {
        // Calculate desired camera position
        glm::vec3 desiredCameraPos = calculateDesiredCameraPosition();

        // Initialize camera position on first frame
        if (!cameraInitialized) {
            currentCameraPos = desiredCameraPos;
            cameraInitialized = true;
        }

        // Smooth follow with lerp (note: this is frame-rate dependent,
        // but acceptable for camera smoothing)
        float followSpeed = cameraFollowSpeed * 0.016f; // Approximate 60fps
        currentCameraPos = glm::mix(currentCameraPos, desiredCameraPos,
                                   glm::clamp(followSpeed, 0.0f, 1.0f));

        // Clamp distance from player
        glm::vec3 toCamera = currentCameraPos - position;
        float dist = glm::length(toCamera);
        if (dist > cameraMaxDistance) {
            currentCameraPos = position + glm::normalize(toCamera) * cameraMaxDistance;
        } else if (dist < cameraMinDistance && dist > 0.001f) {
            currentCameraPos = position + glm::normalize(toCamera) * cameraMinDistance;
        }

        // Look at player with slight upward offset
        glm::vec3 lookAt = position + glm::vec3(0.0f, 0.3f, 0.0f);
        return glm::lookAt(currentCameraPos, lookAt, glm::vec3(0.0f, 1.0f, 0.0f));
    }

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
