#pragma once
#include <SDL2/SDL.h>
#include "utils/spritesheet.h"
#include "utils/tilemap.h" // Include Tilemap for the update signature
#include "utils/collisions_defs.h" // Include collision definitions

class Entity {
public:
    Entity(SDL_Renderer* renderer, const char* sprite_path, int sprite_width,
           int sprite_height, float x, float y, int** animations);
    virtual ~Entity();

    // Update signature now includes Tilemap for collision checks
    virtual void update(Tilemap* map, float time, float deltaTime) = 0;
    void render(SDL_Renderer* renderer);

    SDL_Point getPosition() const;
    SDL_FRect getBoundingBox() const; // Use SDL_FRect for float precision

    void setAnimations(int** animations);
    void setPosition(float new_x, float new_y); // Use float for position
    void setAnimation(int animation_index);
    void setStage(int stage_index);
    bool advanceAnimation();
    void setFlipped(bool flip);

    void setSpriteSheet(Spritesheet* sheet);
    void setSpriteSize(int width, int height);

    virtual void markForDeletion();
    virtual bool isMarkedForDeletion() const;

    // Public members for easier access, consider getters/setters if needed
    float x, y;
    float vx = 0.0f; // Velocity x
    float vy = 0.0f; // Velocity y
    int spriteWidth, spriteHeight;

    CollisionLayer layer = CollisionLayer::NONE; // What this entity IS
    CollisionLayer mask = CollisionLayer::NONE;  // What this entity COLLIDES WITH

protected:
    Spritesheet* spritesheet;
    int currentStage;
    int currentAnimation;
    bool flipped;
    int** animations; // Consider std::vector<std::vector<int>>

    bool markedForDeletion;
};
