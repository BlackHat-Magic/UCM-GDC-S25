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

void MovementAttackAnimated::update(Tilemap *map, float time, float deltaTime) {
    Direction newDirection = control(map, time, deltaTime);
    // did this frame take longer than the entire animation?
    if (time - lastAnimationTime >= animationSpeed) {
        lastAnimationTime = time;
        if (advanceAnimation()) {
            if (isAttacking) {
                isAttacking = false;
                setAnimation(0);
                return;
            }
        }
    }

    // flipping logic
    if (newDirection != direction) {
        setStage(0);
        if (newDirection != NONE) {
            setAnimation(1);

            if (newDirection == LEFT || newDirection == UP_LEFT || newDirection == DOWN_LEFT) {
                setFlipped(false);
            } else if (newDirection == RIGHT || newDirection == UP_RIGHT || newDirection == DOWN_RIGHT) {
                setFlipped(true);
            }
        } else {
            setAnimation(0);
        }
    }

    float proposedX = x + vx * deltaTime;
    float proposedY = y + vy * deltaTime;

    if (map->intersects_rect(proposedX + spriteWidth / 2, proposedY + spriteWidth / 2, spriteWidth, spriteHeight) == NONE) {
        x = proposedX;
        y = proposedY;
    } else {
        if (map->intersects_rect(proposedX, y, spriteWidth, spriteHeight) == NONE) {
            x = proposedX;
        }
    
        if (map->intersects_rect(x, proposedY, spriteWidth, spriteHeight) == NONE) {
            y = proposedY;
        }
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
