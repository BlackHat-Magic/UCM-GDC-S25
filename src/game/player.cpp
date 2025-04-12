#include "player.h"
#include  <SDL2/SDL.h>

const int PLAYER_SPEED = 5;

Player::Player(SDL_Renderer* renderer, const InputHandler* input_handler, float x, float y)
    : MovementAttackAnimated(renderer, "assets/sprites/arcanist.png", 24, 24, x, y, nullptr, 0.1f, PLAYER_SPEED),
      input_handler(input_handler) {
    int* idle_animation = new int[2]{ 0, -1 };
    int* walk_animation = new int[7]{ 1, 2, 3, 4, 5, 6, -1 };
    int* attack_animation = new int[3]{ 4, 5, -1 };
    
    int** animations = new int*[3];
    animations[0] = idle_animation;
    animations[1] = walk_animation;
    animations[2] = attack_animation;

    setAnimations(animations);
}

MovementDirection Player::control(float time, float deltaTime) {
    MovementDirection direction = NONE;

    if (input_handler->is_key_pressed(SDL_SCANCODE_UP)) {
        direction = UP;
    } else if (input_handler->is_key_pressed(SDL_SCANCODE_DOWN)) {
        direction = DOWN;
    } else if (input_handler->is_key_pressed(SDL_SCANCODE_LEFT)) {
        direction = LEFT;
    } else if (input_handler->is_key_pressed(SDL_SCANCODE_RIGHT)) {
        direction = RIGHT;
    }

    if (input_handler->is_mouse_button_pressed(SDL_BUTTON_LEFT)) {
        attack(time);
    }

    return direction;
}