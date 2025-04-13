#pragma once
#include "entity.h"
#include "../utils/tilemap.h"
#include <cmath> // For std::abs

// animations[0] = idle, animations[1] = movement, animations[2] = attack
class MovementAttackAnimated : public Entity {
public:
    MovementAttackAnimated(
        SDL_Renderer* renderer, const char* sprite_path, int sprite_width,
        int sprite_height, float x, float y, int** animations,
        float animation_speed, float movement_speed
    );

    void update(Tilemap* map, float time, float deltaTime) override;
    // Control now modifies vx, vy directly, doesn't return Direction
    virtual void control(Tilemap* map, float time, float deltaTime) = 0;

    void attack(float time); // Trigger attack animation

    void setAnimationSpeed(float speed);
    void setMovementSpeed(float speed);
    float getAnimationSpeed() const;
    float getMovementSpeed() const;

protected: // Make accessible to derived classes if needed, else private
    float lastAnimationTime = 0.0f;
    float animationSpeed;
    float movementSpeed;
    bool isAttacking = false; // Track if attack animation is playing
};
