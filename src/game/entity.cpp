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
        animations(animations) {
            spritesheet = new Spritesheet(
                renderer, 
                sprite_path, 
                sprite_width, 
                sprite_height
            );
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
        // I am not a fan but ig it works
        for (int i = 0; animations[i] != nullptr; ++i) {
            delete[] animations[i];  // Free each animation track (int*)
        }
        delete[] animations;  // Free the array of pointers
    }
    animations = new_animations;
}

void Entity::render(SDL_Renderer* renderer) {
    if (!spritesheet || !animations || !animations[currentAnimation]) return;

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
    currentAnimation = animation_index;
    currentStage = 0;
}

void Entity::setStage(int stage_index) {
    currentStage = stage_index;
}

void Entity::setFlipped(bool flip) {
    flipped = flip;
}

bool Entity::advanceAnimation() {
    if (!animations || !animations[currentAnimation])
        return false;

    ++currentStage;
    if (animations[currentAnimation][currentStage] < 0) {
        currentStage = 0;
        return true;
    }
    return false;
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
