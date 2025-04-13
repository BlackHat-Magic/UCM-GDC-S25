#include "entity.h"
#include <cstdlib>
#include <iostream>
#include <vector> // Include vector if switching animations type

Entity::Entity(SDL_Renderer* renderer, const char* sprite_path, int sprite_width,
               int sprite_height, float x, float y, int** animations) :
    x(x),
    y(y),
    vx(0.0f), // Initialize velocity
    vy(0.0f), // Initialize velocity
    spriteWidth(sprite_width),
    spriteHeight(sprite_height),
    layer(CollisionLayer::NONE), // Default layer
    mask(CollisionLayer::NONE),  // Default mask
    flipped(false),
    currentStage(0),
    currentAnimation(0),
    animations(animations), // Raw pointer management - careful!
    markedForDeletion(false),
    spritesheet(nullptr) // Initialize spritesheet to nullptr
{
    if (sprite_path) {
        try {
            spritesheet = new Spritesheet(
                renderer, sprite_path, sprite_width, sprite_height
            );
        } catch (const std::runtime_error& e) {
            std::cerr << "Error creating spritesheet: " << e.what() << std::endl;
            // Handle error appropriately, maybe mark entity for deletion or use a default?
            spritesheet = nullptr;
        }
    }
}

Entity::~Entity() {
    delete spritesheet; // Safe even if nullptr

    // --- Memory Management for animations ---
    // This raw pointer management is risky. Consider using
    // std::vector<std::vector<int>> and passing by value/reference,
    // or using smart pointers if ownership semantics are complex.
    if (animations) {
        for (int i = 0; animations[i] != nullptr; ++i) {
            delete[] animations[i]; // Free each animation track (int*)
        }
        delete[] animations; // Free the array of pointers
    }
}

void Entity::setAnimations(int** new_animations) {
    // Clean up old animations first
    if (animations) {
        for (int i = 0; animations[i] != nullptr; ++i) {
            delete[] animations[i];
        }
        delete[] animations;
    }
    animations = new_animations;
    // Reset animation state
    currentAnimation = 0;
    currentStage = 0;
}

void Entity::render(SDL_Renderer* renderer) {
    if (!spritesheet || !animations || !animations[currentAnimation]) return;

    // Determine number of stages in current animation safely
    int stageCount = 0;
    if (animations[currentAnimation]) {
        while (animations[currentAnimation][stageCount] >= 0) {
            stageCount++;
        }
    }

    if (stageCount == 0) return; // No valid stages in this animation

    // Ensure currentStage is valid
    if (currentStage >= stageCount) {
        currentStage = 0; // Wrap around if needed
    }

    int sprite_index = animations[currentAnimation][currentStage];
    if (sprite_index < 0) {
        // This case should ideally not happen if stageCount is calculated correctly
        // Maybe default to first frame or log an error
        // currentStage = 0;
        // sprite_index = animations[currentAnimation][currentStage];
        // if (sprite_index < 0) return; // Still invalid? Bail out.
        return;
    }


    spritesheet->select_sprite(sprite_index);
    SDL_RendererFlip flip = flipped ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE;
    // Draw using float coordinates for position
    spritesheet->draw(renderer, x, y, spriteWidth, spriteHeight, flip);
}

SDL_Point Entity::getPosition() const {
    // Returns integer point, might lose precision
    return SDL_Point{static_cast<int>(x), static_cast<int>(y)};
}

SDL_FRect Entity::getBoundingBox() const {
    // Return float rect centered on x, y
    return SDL_FRect{
        x - spriteWidth / 2.0f,
        y - spriteHeight / 2.0f,
        static_cast<float>(spriteWidth),
        static_cast<float>(spriteHeight)};
}


void Entity::setPosition(float new_x, float new_y) {
    x = new_x;
    y = new_y;
}

void Entity::setAnimation(int animation_index) {
    // Bounds check
    int count = 0;
    if (animations) {
        while (animations[count] != nullptr) {
            count++;
        }
    }

    if (animation_index >= 0 && animation_index < count) {
        if (currentAnimation != animation_index) { // Only reset stage if animation changes
            currentAnimation = animation_index;
            currentStage = 0;
        }
    } else {
        // Handle invalid index - maybe default to 0 or log error
        if (count > 0) { // Only default if animations exist
             if (currentAnimation != 0) {
                 currentAnimation = 0;
                 currentStage = 0;
             }
        } else {
            // No animations available
            currentAnimation = 0; // Or perhaps an invalid state like -1
            currentStage = 0;
        }
    }
}

void Entity::setStage(int stage_index) {
    if (!animations || !animations[currentAnimation]) {
        return;
    }

    // Bounds check
    int stageCount = 0;
     while (animations[currentAnimation][stageCount] >= 0) {
        stageCount++;
    }

    if (stage_index >= 0 && stage_index < stageCount) {
        currentStage = stage_index;
    }
    // else: Do nothing if index is out of bounds, or clamp/wrap?
}

void Entity::setFlipped(bool flip) {
    flipped = flip;
}

bool Entity::advanceAnimation() {
    if (!animations || !animations[currentAnimation]) return false;

    int stageCount = 0;
     while (animations[currentAnimation][stageCount] >= 0) {
        stageCount++;
    }
    if (stageCount == 0) return false; // No stages to advance

    ++currentStage;
    if (currentStage >= stageCount || animations[currentAnimation][currentStage] < 0) {
        currentStage = 0; // Loop animation
        return true;      // Indicate animation looped/finished
    }
    return false; // Indicate animation is still progressing
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

void Entity::markForDeletion() {
    markedForDeletion = true;
}
bool Entity::isMarkedForDeletion() const {
    return markedForDeletion;
}
