#include "movement_attack_animated.h"
#include <cmath> // For std::abs, std::sqrt
#include <iostream> // For debugging

MovementAttackAnimated::MovementAttackAnimated(
    SDL_Renderer* renderer, const char* sprite_path, int sprite_width,
    int sprite_height, float x, float y, int** animations,
    float animation_speed, float movement_speed
) :
    Entity(
        renderer, sprite_path, sprite_width, sprite_height, x, y, animations
    ),
    animationSpeed(animation_speed),
    movementSpeed(movement_speed),
    isAttacking(false) // Ensure isAttacking starts false
{
    // Initial animation state often set by derived class
    // setAnimation(0);
    // setStage(0);
}

void MovementAttackAnimated::update(Tilemap* map, float time, float deltaTime) {
    // 1. Determine desired velocity from derived class logic
    control(map, time, deltaTime); // Sets vx, vy

    // 2. Handle Animation Timing and State Transitions
    bool animationLooped = false;
    if (animationSpeed > 0 && time - lastAnimationTime >= animationSpeed) {
        lastAnimationTime = time;
        animationLooped = advanceAnimation(); // Returns true if animation looped
    }

    // Animation State Logic
    const float moveThreshold = 0.1f; // Minimum speed to trigger walk animation
    if (isAttacking) {
        setAnimation(2); // Ensure attack animation is set
        if (animationLooped) { // Attack animation finished
            isAttacking = false;
            // Immediately decide next state based on current velocity
            if (std::abs(vx) > moveThreshold || std::abs(vy) > moveThreshold) {
                setAnimation(1); // Start walking animation
            } else {
                setAnimation(0); // Go back to idle
            }
        }
    } else {
        // Not attacking: Choose idle or walk based on velocity
        if (std::abs(vx) > moveThreshold || std::abs(vy) > moveThreshold) {
            setAnimation(1); // Walking animation
            // Flipping based on horizontal velocity
            if (vx > moveThreshold) {
                setFlipped(true); // Facing right
            } else if (vx < -moveThreshold) {
                setFlipped(false); // Facing left
            }
            // If vx is near zero, maintain current flip state
        } else {
            setAnimation(0); // Idle animation
        }
    }

    // 3. Movement and Collision with Tilemap
    float proposedX = x + vx * deltaTime;
    float proposedY = y + vy * deltaTime;

    // --- Basic Tilemap Collision (Placeholder - Needs Improvement) ---
    // This needs to be replaced with a better system using layers, masks,
    // and potentially sliding based on collision normals.
    // The current map->intersects_rect is likely insufficient.

    // Example: Simple AABB check with basic slide
    SDL_FRect currentBox = getBoundingBox();
    SDL_FRect proposedBoxX = {proposedX - spriteWidth / 2.0f, y - spriteHeight / 2.0f, currentBox.w, currentBox.h};
    SDL_FRect proposedBoxY = {x - spriteWidth / 2.0f, proposedY - spriteHeight / 2.0f, currentBox.w, currentBox.h};
    SDL_FRect proposedBoxCombined = {proposedX - spriteWidth / 2.0f, proposedY - spriteHeight / 2.0f, currentBox.w, currentBox.h};

    bool collisionX = map->checkCollision(proposedBoxX, mask); // Needs Tilemap::checkCollision implementation
    bool collisionY = map->checkCollision(proposedBoxY, mask);
    bool collisionCombined = map->checkCollision(proposedBoxCombined, mask);


    if (!collisionCombined) { // No collision with combined movement
         x = proposedX;
         y = proposedY;
    } else {
        // Try moving only on X axis
        if (!collisionX) {
            x = proposedX;
        } else {
             vx = 0; // Stop horizontal movement on collision
        }
        // Try moving only on Y axis
        if (!collisionY) {
            y = proposedY;
        } else {
             vy = 0; // Stop vertical movement on collision
        }
    }

    // Update position (might be redundant if x,y are directly used)
    // setPosition(x, y);
}


void MovementAttackAnimated::attack(float time) {
    if (!isAttacking) {
        isAttacking = true;
        setAnimation(2); // Switch to attack animation
        setStage(0);     // Start from the beginning
        lastAnimationTime = time; // Reset animation timer for attack
    }
}

void MovementAttackAnimated::setAnimationSpeed(float speed) {
    animationSpeed = speed;
}

void MovementAttackAnimated::setMovementSpeed(float speed) {
    movementSpeed = speed;
}

float MovementAttackAnimated::getAnimationSpeed() const {
    return animationSpeed;
}

float MovementAttackAnimated::getMovementSpeed() const {
    return movementSpeed;
}
