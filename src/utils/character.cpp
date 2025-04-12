#include "character.h"
#include  <SDL2/SDL.h>

const int CHARACTER_SPEED = 5;

Character::Character ( SDL_Renderer* renderer, const char* spritePath,
    int spriteWidth, int spriteHeight, int x, int y)
    : velocityX (0), velocityY (0) {
    // create spritesheet
    spritesheet = new Spritesheet (renderer, spritePath, spriteWidth, spriteHeight);

    dstRect.x = x;
    dstRect.y = y;
    dstRect.w = spriteWidth;
    dstRect.h = spriteHeight;
}

Character::~Character () {
    delete spritesheet;
}

void Character::handleEvent (const SDL_Event& e) {
    if (e.type == SDL_KEYDOWN && !e.key.repeat) {
        switch (e.key.keysym.sym) {
            case SDLK_LEFT:
                velocityX = -1 * CHARACTER_SPEED;
                break;
            case SDLK_RIGHT:
                velocityX = CHARACTER_SPEED;
                break;
            case SDLK_UP:
                velocityY = -1 * CHARACTER_SPEED;
                break;
            case SDLK_DOWN:
                velocityY = CHARACTER_SPEED;
                break;
            default:
                break;
        }
    } else if (e.type == SDL_KEYUP && !e.key.repeat) {
        switch (e.key.keysym.sym) {
            case SDLK_LEFT:
            case SDLK_RIGHT:
                velocityX = 0;
                break;
            case SDLK_UP:
            case SDLK_DOWN:
                velocityY = 0;
                break;
            default:
                break;
        }
    }
}

void Character::update (float deltaTime) {
    dstRect.x += static_cast <int> (velocityX * deltaTime);
    dstRect.y += static_cast <int> (velocityY * deltaTime);

    if (velocityX != 0 || velocityY != 0) {
        static int frame = 0;
        frame = (frame + 1) % 4;
        spritesheet->select_sprite (frame);
    }
}

void Character::render (SDL_Renderer* renderer) {
    spritesheet->draw (renderer, dstRect.x, dstRect.y, dstRect.w, dstRect.h);
}

SDL_Point Character::getPosition () const {
    return {dstRect.x, dstRect.y};
}