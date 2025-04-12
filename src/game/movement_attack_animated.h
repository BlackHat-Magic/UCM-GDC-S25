#pragma once
#include "entity.h"

enum MovementDirection {
    UP,
    UP_LEFT,
    UP_RIGHT,
    DOWN,
    DOWN_LEFT,
    DOWN_RIGHT,
    LEFT,
    RIGHT,
    NONE
};

// animations[0] = idle, animations[i] = movement, animations[2] = attack
class MovementAttackAnimated : public Entity {
public:
    MovementAttackAnimated(SDL_Renderer* renderer, const char* sprite_path, int sprite_width, int sprite_height, float x, float y, int** animations, float animation_speed, float movement_speed);

    void update(float time, float deltaTime) override;
    virtual MovementDirection control(float time, float deltaTime) = 0;
    void attack(float time);
    void setAnimationSpeed(float speed);
    void setMovementSpeed(float speed);
    float getAnimationSpeed() const;
    float getMovementSpeed() const;

private:
    float lastAnimationTime;
    MovementDirection direction;
    bool isAttacking; // used to know not to change back to idle/movement animation during attack
protected:
    float animationSpeed;
    float movementSpeed;
};