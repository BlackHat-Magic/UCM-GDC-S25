#pragma once
#include <SDL2/SDL.h>
#include "utils/spritesheet.h"
#include "utils/input.h"
#include "movement_attack_animated.h"

class Player : public MovementAttackAnimated {
public:
    Player(SDL_Renderer* renderer, const InputHandler* input_handler, float x, float y, float initial_health = 100.0f);

    void control(Tilemap *_map, float time, float deltaTime) override;
    void takeDamage (float amount);
    bool isAlive () const;
    float getHealth () const;
    float getMaxHealth () const;
private:
    const InputHandler* input_handler;
    float health;
    float maxHealth;
};