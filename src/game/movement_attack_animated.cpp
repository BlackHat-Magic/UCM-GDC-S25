#include "movement_attack_animated.h"

MovementAttackAnimated::MovementAttackAnimated(SDL_Renderer* renderer, const char* sprite_path, int sprite_width, int sprite_height, float x, float y, int** animations, float animation_speed, float movement_speed)
    : Entity(renderer, sprite_path, sprite_width, sprite_height, x, y, animations),
      animationSpeed(animation_speed),
      movementSpeed(movement_speed),
      direction(NONE),
      isAttacking(false) {
    setAnimation(0);
    setStage(0);
    lastAnimationTime = 0.0f;
}

void MovementAttackAnimated::update(float time, float deltaTime) {
    MovementDirection newDirection = control(time, deltaTime);

    if (time - lastAnimationTime >= animationSpeed) {
        lastAnimationTime = time;
        if (advanceAnimation()) {
            isAttacking = false;
            if (isAttacking) {
                setAnimation(0);
                return;
            }
        }
    }

    if (newDirection != direction) {
        setStage(0);
        if (newDirection != NONE) {
            setAnimation(1);
        } else {
            setAnimation(0);
        }
    }

    direction = newDirection;

    switch (direction) {
        case UP:    y -= movementSpeed * deltaTime; break;
        case DOWN:  y += movementSpeed * deltaTime; break;
        case LEFT:  x -= movementSpeed * deltaTime; break;
        case RIGHT: x += movementSpeed * deltaTime; break;
        case UP_LEFT:   x -= movementSpeed * deltaTime; y -= movementSpeed * deltaTime; break;
        case UP_RIGHT:  x += movementSpeed * deltaTime; y -= movementSpeed * deltaTime; break;
        case DOWN_LEFT: x -= movementSpeed * deltaTime; y += movementSpeed * deltaTime; break;
        case DOWN_RIGHT: x += movementSpeed * deltaTime; y += movementSpeed * deltaTime; break;
        default: break;
    }

    setPosition(static_cast<int>(x), static_cast<int>(y));
}

void MovementAttackAnimated::attack(float time) {
    if (!isAttacking) {
        isAttacking = true;
        setAnimation(2); 
        setStage(0);
        lastAnimationTime = time;
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
