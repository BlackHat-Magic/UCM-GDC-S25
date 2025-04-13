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
    Direction newDirection = control(time, deltaTime);

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

    direction = newDirection;

    float dx = 0.0f;
    float dy = 0.0f;

    switch (direction) {
        case UP:
            dy = -movementSpeed * deltaTime;
            break;
        case DOWN:
            dy = movementSpeed * deltaTime;
            break;
        case LEFT:
            dx = -movementSpeed * deltaTime;
            break;
        case RIGHT:
            dx = movementSpeed * deltaTime;
            break;
        case UP_LEFT:
            dx = -movementSpeed * deltaTime / std::sqrt(2);
            dy = -movementSpeed * deltaTime / std::sqrt(2);
            break;
        case UP_RIGHT:
            dx = movementSpeed * deltaTime / std::sqrt(2);
            dy = -movementSpeed * deltaTime / std::sqrt(2);
            break;
        case DOWN_LEFT:
            dx = -movementSpeed * deltaTime / std::sqrt(2);
            dy = movementSpeed * deltaTime / std::sqrt(2);
            break;
        case DOWN_RIGHT:
            dx = movementSpeed * deltaTime / std::sqrt(2);
            dy = movementSpeed * deltaTime / std::sqrt(2);
            break;
        default:
            break;
    }

    float proposedX = x + dx;
    float proposedY = y + dy;

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

    setPosition(x, y);
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
