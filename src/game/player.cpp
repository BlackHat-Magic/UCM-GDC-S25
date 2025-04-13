#include "player.h"
#include  <SDL2/SDL.h>

Player::Player(SDL_Renderer* renderer, const InputHandler* input_handler, float x,
            float y, float initial_health)
    : MovementAttackAnimated(renderer, "assets/sprites/arcanist.png", 24, 24, x, y, 
            nullptr, 0.1f, 200.0f),
        input_handler(input_handler),
        health (initial_health),
        maxHealth (initial_health) {
        int* idle_animation = new int[2]{ 0, -1 };
        int* walk_animation = new int[7]{ 1, 2, 3, 4, 5, 6, -1 };
        int* attack_animation = new int[3]{ 4, 5, -1 };
        
        int** player_animations = new int*[4];
        player_animations[0] = idle_animation;
        player_animations[1] = walk_animation;
        player_animations[2] = attack_animation;
        player_animations[3] = nullptr;

        setAnimations (player_animations);
        setAnimation (0);
}

void Player::control(Tilemap *_map, float time, float deltaTime) {
    if (!isAlive ()) {
        return;
    }

    // reset each frame
    vx = 0.0f;
    vy = 0.0f;

    // grab input
    if (input_handler->is_key_pressed (SDL_SCANCODE_LEFT)) vx -= movementSpeed;
    if (input_handler->is_key_pressed (SDL_SCANCODE_RIGHT)) vx += movementSpeed;
    if (input_handler->is_key_pressed (SDL_SCANCODE_UP)) vy -= movementSpeed;
    if (input_handler->is_key_pressed (SDL_SCANCODE_DOWN)) vy += movementSpeed;

    // normalize
    float lenSq = vx*vx + vy*vy;
    if (lenSq > movementSpeed * movementSpeed) {
        float len std::sqrt (lenSq);
        vx = (vx / len) * movementSpeed;
        vy = (vy / len) * movementSpeed;
    }

    if (input_handler->is_mouse_button_pressed (SDL_BUTTON_LEFT)) attack (time);
}

void Player::takeDamage (float amount) {
    // you can't make someone more dead
    if (!isAlive ()) {
        return;
    }

    health -= amount;
    health = std::max (health, 0.0f);

    // maybe do a death animation instead
    if (health <= 0.0f) {
        markForDeletion ();
    }
}

bool Player::isAlive () const {
    return health > 0.0f;
}

float Player::getHealth () const {
    return health;
}

float Player::getMaxHealth () const {
    return maxHealth;
}