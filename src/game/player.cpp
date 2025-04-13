#include "player.h"
#include <SDL2/SDL.h>
#include <cmath> // For std::sqrt

Player::Player(
    SDL_Renderer* renderer, const InputHandler* input_handler, float x, float y,
    float initial_health
) :
    MovementAttackAnimated(
        renderer, "assets/sprites/arcanist.png", 24, 24, x, y, nullptr, 0.1f,
        200.0f
    ), // Pass params to base
    input_handler(input_handler),
    health(initial_health),
    maxHealth(initial_health) {
    // Define animations (Consider loading from file or a config)
    int* idle_animation = new int[2]{0, -1};
    int* walk_animation = new int[7]{1, 2, 3, 4, 5, 6, -1};
    int* attack_animation = new int[3]{4, 5, -1}; // Example attack frames

    int** player_animations = new int*[4];
    player_animations[0] = idle_animation;
    player_animations[1] = walk_animation;
    player_animations[2] = attack_animation;
    player_animations[3] = nullptr; // Null terminator for the array of pointers

    setAnimations(player_animations); // Set the animations in the base class
    setAnimation(0);                  // Start with idle animation

    // Set collision layer and mask
    layer = CollisionLayer::LAYER_GROUND_PLAYER;
    mask = CollisionLayer::MASK_GROUND_PLAYER;
}

void Player::control(Tilemap* map, float time, float deltaTime) {
    if (!isAlive()) {
        vx = 0.0f;
        vy = 0.0f;
        return;
    }

    // Reset velocity each frame based on input
    vx = 0.0f;
    vy = 0.0f;

    // Get input and set velocity components
    if (input_handler->is_key_pressed(SDL_SCANCODE_LEFT)) {
        vx -= movementSpeed;
    }
    if (input_handler->is_key_pressed(SDL_SCANCODE_RIGHT)) {
        vx += movementSpeed;
    }
    if (input_handler->is_key_pressed(SDL_SCANCODE_UP)) {
        vy -= movementSpeed;
    }
    if (input_handler->is_key_pressed(SDL_SCANCODE_DOWN)) {
        vy += movementSpeed;
    }

    // Normalize diagonal movement
    float lenSq = vx * vx + vy * vy;
    if (lenSq > movementSpeed * movementSpeed) {
        float len = std::sqrt(lenSq);
        if (len > 0) { // Avoid division by zero
            vx = (vx / len) * movementSpeed;
            vy = (vy / len) * movementSpeed;
        }
    }

    // Check for attack input
    if (input_handler->is_mouse_button_pressed(SDL_BUTTON_LEFT)) {
        attack(time); // Call base class attack method
    }
}

void Player::takeDamage(float amount) {
    if (!isAlive()) {
        return; // Can't damage dead player
    }

    health -= amount;
    health = std::max(health, 0.0f); // Clamp health at 0

    if (health <= 0.0f) {
        // Consider playing a death animation instead of immediate deletion
        // For now, mark for deletion
        markForDeletion();
        vx = 0; // Stop movement
        vy = 0;
        // Maybe set a specific "dead" animation state? setAnimation(DEATH_ANIM);
    }
}

bool Player::isAlive() const {
    return health > 0.0f;
}

float Player::getHealth() const {
    return health;
}

float Player::getMaxHealth() const {
    return maxHealth;
}
