#include "fireball.h"

Fireball::Fireball(SDL_Renderer* renderer, const char* sprite_path,
                   int sprite_width, int sprite_height, float x, float y,
                   float vx, float vy, Entity* owner, float damage)
    : Entity(renderer, sprite_path, sprite_width, sprite_height, x, y, nullptr),
      vx(vx),
      vy(vy),
      owner(owner),
      damage(damage)
{
    // For a projectile the animation can be as simple as a static sprite.
    // Set any animation state as needed.
    setAnimation(0);
    setStage(0);
}

void Fireball::update(float time, float deltaTime) {
    // Move the fireball along its velocity
    x += vx * deltaTime;
    y += vy * deltaTime;

    // Optionally, you might want to advance an animation sequence.
    advanceAnimation();
}

bool Fireball::isOffScreen(int screenWidth, int screenHeight) const {
    SDL_Point pos = getPosition();
    return pos.x < 0 || pos.x > screenWidth || pos.y < 0 ||
           pos.y > screenHeight;
}
