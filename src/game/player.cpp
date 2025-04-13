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

Direction Player::control(Tilemap *_map, float time, float deltaTime) {
    Direction direction = NONE;

    if (!isAlive ()) {
        return NONE;
    }

    if (input_handler->is_key_pressed(SDL_SCANCODE_UP)) {
        if (input_handler->is_key_pressed(SDL_SCANCODE_LEFT)) {
            direction = UP_LEFT;
        } else if (input_handler->is_key_pressed(SDL_SCANCODE_RIGHT)) {
            direction = UP_RIGHT;
        } else {
            direction = UP;
        }
    } else if (input_handler->is_key_pressed(SDL_SCANCODE_DOWN)) {
        if (input_handler->is_key_pressed(SDL_SCANCODE_LEFT)) {
            direction = DOWN_LEFT;
        } else if (input_handler->is_key_pressed(SDL_SCANCODE_RIGHT)) {
            direction = DOWN_RIGHT;
        } else {
            direction = DOWN;
        }
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