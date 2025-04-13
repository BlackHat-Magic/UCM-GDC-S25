#include "entity.h"
#include <cstdlib>
#include <iostream>

Entity::Entity(SDL_Renderer* renderer, const char* sprite_path, int sprite_width, int
        sprite_height, float x, float y, int** animations) : 
        x(x), y(y), 
        spriteWidth(sprite_width), 
        spriteHeight(sprite_height),
        flipped(false), 
        currentStage(0), 
        currentAnimation(0),
        animations(animations),
        markedForDeletion (false)
        {
            if (sprite_path) {
                spritesheet = new Spritesheet(
                    renderer, 
                    sprite_path, 
                    sprite_width, 
                    sprite_height
                );
            } else {
                spritesheet = nullptr;
            }
        }

Entity::~Entity() {
    delete spritesheet;

    if (animations) {
        for (int i = 0; animations[i] != nullptr; ++i) {
            delete[] animations[i];  // Free each animation track (int*)
        }
        delete[] animations;  // Free the array of pointers
    }
}

void Entity::setAnimations(int** new_animations) {
    if (animations) {
        for (int i = 0; animations[i] != nullptr; ++i) {
            delete[] animations[i];  // Free each animation track (int*)
        }
        delete[] animations;  // Free the array of pointers
    }
    animations = new_animations;
}

void Entity::render(SDL_Renderer* renderer) {
    if (!spritesheet || !animations || !animations[currentAnimation]) return;

    // make sure we don't overflow animation stages
    int stageCount = 0;
    while (animations[currentAnimation][stageCount] >= 0) {
        stageCount++;
    }
    if (currentStage >= stageCount) {
        currentStage = 0;
    }

    int sprite_index = animations[currentAnimation][currentStage];
    if (sprite_index < 0) return;

    spritesheet->select_sprite(sprite_index);
    SDL_RendererFlip flip = flipped ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE;
    spritesheet->draw(renderer, static_cast<int>(x), static_cast<int>(y), spriteWidth, spriteHeight, flip);
}

SDL_Point Entity::getPosition() const {
    return SDL_Point{ static_cast<int>(x), static_cast<int>(y) };
}

void Entity::setPosition(int new_x, int new_y) {
    x = static_cast<float>(new_x);
    y = static_cast<float>(new_y);
}

void Entity::setAnimation(int animation_index) {
    // bounds check
    int count = 0;
    if(animations) {
        while (animations[count] != nullptr) {
            count++;
        }
    }
    if (animation_index >= 0 && animation_index < count) {
        currentAnimation = animation_index;
        currentStage = 0;
    } else {
        // either we have no animations or they are defining out of bounds
        currentAnimation = 0;
        currentStage = 0;
    }
}

void Entity::setStage(int stage_index) {
    // if no anumations, don't bother
    if (!animations) {
        return;
    }
    if (!animations[currentAnimation]) {
        return;
    }

    // bounds check
    int stageCount = 0;
    while (animations[currentAnimation][stageCount] >= 0) {
        stageCount++;
    }
    if (stage_index >= 0 && stage_index < stageCount) {
        currentStage = stage_index;
    } else {
        // out of bounds
        // currentStage = 0;
    }
}

void Entity::setFlipped(bool flip) {
    flipped = flip;
}

bool Entity::advanceAnimation() {
    if (!animations || !animations[currentAnimation])
        return false;

    ++currentStage;
    if (animations[currentAnimation][currentStage] < 0) {
        currentStage = 0; // loop
        return true;
    }
    return false; //animation is finished
}

void Entity::setSpriteSheet(Spritesheet* sheet) {
    if (spritesheet) {
        delete spritesheet;
    }
    spritesheet = sheet;
}

void Entity::setSpriteSize(int width, int height) {
    spriteWidth = width;
    spriteHeight = height;
}

void Entity::markForDeletion () {
    markedForDeletion = true;
}
bool Entity::isMarkedForDeletion () const {
    return markedForDeletion;
}