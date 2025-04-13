#pragma once
#include "entity.h"
#include "utils/spritesheet.h"
#include <SDL2/SDL.h>
#include <cmath>

class Fireball : public Entity {
public:
    Fireball(SDL_Renderer* renderer, const char* sprite_path, int sprite_width,
             int sprite_height, float x, float y, float vx, float vy,
             Entity* owner, float damage);
    // ~Fireball();

    // For the projectile, update its position using its velocity.
    void update(Tilemap *_map, float time, float deltaTime, float cameraX, float cameraY) override;

    // Test if the projectile is off-screen.
    bool isOffScreen(int screenWidth, int screenHeight, float cameraX, float cameraY) const;

    Entity* getOwner() const { return owner; }
    float getDamage() const { return damage; }

private:
    float vx, vy;
    Entity* owner; // pointer to the entity that fired the projectile
    float damage;
};
