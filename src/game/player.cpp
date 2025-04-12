#include "player.h"
#include  <SDL2/SDL.h>

const int PLAYER_SPEED = 5;

Player::Player ( SDL_Renderer* renderer, const char* spritePath,
    int spriteWidth, int spriteHeight, int x, int y)
    : velocityX (0), velocityY (0) {
    // create spritesheet
    spritesheet = new Spritesheet (renderer, spritePath, spriteWidth, spriteHeight);

    dstRect.x = x;
    dstRect.y = y;
    dstRect.w = spriteWidth;
    dstRect.h = spriteHeight;
}

Player::~Player () {
    delete spritesheet;
}

void Player::update (float deltaTime, const InputHandler& inputHandler) {
    dstRect.x += static_cast <int> (velocityX * deltaTime);
    dstRect.y += static_cast <int> (velocityY * deltaTime);

    if (inputHandler.is_key_pressed(SDL_SCANCODE_LEFT)) {
        velocityX = -1 * PLAYER_SPEED;
      }
    if (inputHandler.is_key_pressed(SDL_SCANCODE_RIGHT)) {
        velocityX = PLAYER_SPEED;
    }
    if (inputHandler.is_key_pressed(SDL_SCANCODE_UP)) {
        velocityY = -1 * PLAYER_SPEED;
    }
    if (inputHandler.is_key_pressed(SDL_SCANCODE_DOWN)) {
        velocityY = PLAYER_SPEED;
    }

    dstRect.x += static_cast<int>(velocityX * deltaTime);
    dstRect.y += static_cast<int>(velocityY * deltaTime);

    if (velocityX != 0 || velocityY != 0) {
        static int frame = 0;
        frame = (frame + 1) % 4;
        spritesheet->select_sprite (frame);
    }
}

void Player::render (SDL_Renderer* renderer) {
    spritesheet->draw (renderer, dstRect.x, dstRect.y, dstRect.w, dstRect.h);
}

SDL_Point Player::getPosition () const {
    return {dstRect.x, dstRect.y};
}