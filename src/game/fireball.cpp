#include "fireball.h"
#include "player.h" // Needed for dynamic_cast to check owner type

Fireball::Fireball(
    SDL_Renderer* renderer, const char* sprite_path, int sprite_width,
    int sprite_height, float x, float y, float initial_vx, float initial_vy,
    Entity* owner, float damage
) :
    Entity(
        renderer, sprite_path, sprite_width, sprite_height, x, y, nullptr
    ), // Pass basic params to Entity
    owner(owner),
    damage(damage) {
    // Set initial velocity in the base Entity members
    this->vx = initial_vx;
    this->vy = initial_vy;

    // Define a simple static animation (frame 0)
    int* static_frame = new int[2]{0, -1}; // Frame 0, then end marker
    int** fireball_animations = new int*[2];
    fireball_animations[0] = static_frame;
    fireball_animations[1] = nullptr; // Null terminator for the array

    setAnimations(fireball_animations);
    setAnimation(0);
    setStage(0);

    // Set collision layer and mask based on owner
    if (dynamic_cast<Player*>(owner)) {
        layer = CollisionLayer::LAYER_PLAYER_PROJECTILE;
        mask = CollisionLayer::MASK_PLAYER_PROJECTILE;
    } else { // Assume enemy owner if not player
        layer = CollisionLayer::LAYER_ENEMY_PROJECTILE;
        mask = CollisionLayer::MASK_ENEMY_PROJECTILE;
    }
}

void Fireball::update(Tilemap* map, float time, float deltaTime) {
    // Basic movement - no tile collision check for fireballs yet
    // A more robust system might check tile collisions based on the mask
    float proposedX = x + vx * deltaTime;
    float proposedY = y + vy * deltaTime;

    // Check tile collision (optional for fireballs, depends on game design)
    SDL_FRect proposedBox = {proposedX - spriteWidth / 2.0f, proposedY - spriteHeight / 2.0f, (float)spriteWidth, (float)spriteHeight};
    if (map->checkCollision(proposedBox, mask)) {
        // Fireball hit a wall/obstacle it cares about
        markForDeletion(); // Destroy the fireball
        return; // Stop further updates this frame
    }

    // If no collision, update position
    x = proposedX;
    y = proposedY;


    // Advance "animation" for static sprite (optional)
    if (time - lastAnimationTime >= 0.1f) { // Arbitrary interval
        advanceAnimation(); // Will just loop frame 0
        lastAnimationTime = time;
    }
}

bool Fireball::isOffScreen(int screenWidth, int screenHeight) const {
    // Check against screen bounds (assuming x,y is center)
    float halfW = spriteWidth / 2.0f;
    float halfH = spriteHeight / 2.0f;
    return (x + halfW < 0 || x - halfW > screenWidth || y + halfH < 0 ||
            y - halfH > screenHeight);
}
