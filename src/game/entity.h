#pragma once
#include <SDL2/SDL.h>
#include "../utils/spritesheet.h"
#include "../utils/tilemap.h"

class Entity {
public:
    Entity(SDL_Renderer* renderer, const char* sprite_path, int sprite_width, int sprite_height, float x, float y, int** animations);
    virtual ~Entity();

    virtual void update(Tilemap *map, float time, float deltaTime, float CameraX, float CameraY) = 0;
    void render(SDL_Renderer* renderer, float cameraX, float cameraY);

    SDL_Point getPosition() const;
    void setAnimations(int** animations);
    void setPosition(int x, int y);
    void setAnimation(int animation_index);
    void setStage(int stage_index);
    bool advanceAnimation();
    void setFlipped(bool flip);

    void setSpriteSheet(Spritesheet* sheet);
    void setSpriteSize(int width, int height);
    float x, y;

    Spritesheet* spritesheet;
    int spriteWidth, spriteHeight;
    int currentStage;
    int currentAnimation;
    bool flipped;
    int** animations;
};