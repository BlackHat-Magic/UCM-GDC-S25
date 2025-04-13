#include "fireball.h"

Fireball::Fireball(SDL_Renderer* renderer, const char* sprite_path,
        int sprite_width, int sprite_height, float x, float y,
        float vx, float vy, Entity* owner, float damage)
    : Entity(renderer, sprite_path, sprite_width, sprite_height, x, y, nullptr),
        vx(vx),
        vy(vy),
        owner(owner),
        damage(damage) {
    int* static_frame = new int[2]{0, -1};
    int** fireball_animations = new int*[2];
    fireball_animations[0] = static_frame;
    fireball_animations[1] = nullptr;
    
    setAnimations (fireball_animations);
    setAnimation(0);
    setStage(0);
}

void Fireball::update(Tilemap *_map, float time, float deltaTime) {
    // Move the fireball along its velocity
    x += vx * deltaTime;
    y += vy * deltaTime;

    // allegedly good practice to advance frames even for static sprites
    if (time - lastAnimationTime >= 0.1f) {
        advanceAnimation ();
        lastAnimationTime = time;
    }
}

bool Fireball::isOffScreen(int screenWidth, int screenHeight) const {
    SDL_Point pos = getPosition();
    return x < -spriteWidth || x > screenWidth || y < -spriteHeight || y > screenHeight;
}
