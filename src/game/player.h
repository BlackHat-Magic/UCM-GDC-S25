#pragma once
#include <SDL2/SDL.h>
#include "utils/spritesheet.h"
#include "utils/input.h"

class Player {
public:
    Player (SDL_Renderer* renderer, const char* spritePath, int spriteWidth, int spriteHeight, int x, int y);
    ~Player ();

    void update (float deltaTime, const InputHandler& inputHandler);

    void render (SDL_Renderer* renderer);

    SDL_Point getPosition () const;

private:
    Spritesheet* spritesheet;
    SDL_Rect dstRect;
    int velocityX;
    int velocityY;
};