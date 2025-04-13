#pragma once
#include "entity.h"
#include "utils/spritesheet.h" // Included via entity.h
#include <SDL2/SDL.h>
#include <cmath>

class Fireball : public Entity {
public:
    Fireball(
        SDL_Renderer* renderer, const char* sprite_path, int sprite_width,
        int sprite_height, float x, float y, float initial_vx, float initial_vy,
        Entity* owner, float damage
    );

    // Update moves based on velocity, checks for tile collision
    void update(Tilemap* map, float time, float deltaTime) override;

    // Test if the projectile is off-screen.
    bool isOffScreen(int screenWidth, int screenHeight) const;

    Entity* getOwner() const { return owner; }
    float getDamage() const { return damage; }

private:
    // Velocity (vx, vy) is now inherited from Entity
    Entity* owner; // Pointer to the entity that fired the projectile
    float damage;
    float lastAnimationTime = 0.0f; // For static sprite "animation"
};
