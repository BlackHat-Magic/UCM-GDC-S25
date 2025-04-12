#pragma once
#include <SDL2/SDL.h>
#include "utils/spritesheet.h"

class Character {
public:
    Character (SDL_Renderer* renderer, const char* spritePath, int spriteWidth, int spriteHeight, int x, int y);
    ~Character ();

    void update (float deltaTime);

    void render (SDL_Renderer* renderer);

    void handleEvent (const SDL_Event& e);

    SDL_Point getPosition () const;

private:
    Spritesheet* spritesheet;
    SDL_Rect dstRect;
    int velocityX;
    int velocityY;
};