#pragma once
#include "movement_attack_animated.h"
#include "fireball.h"
#include "entity.h"
#include <vector>
#include <random>

// Define states for the Geezer
enum GeezerState {
    G_IDLE,
    G_CHASE,
    G_APPROACH,
    G_ATTACK,
    G_WITHDRAW,
    G_FLEE
};

class Geezer : public MovementAttackAnimated {
public:
    // The Geezer takes a pointer to its target (e.g., the player)
    Geezer(SDL_Renderer* renderer, const char* sprite_path, int sprite_width,
           int sprite_height, float x, float y, int** animations,
           float animation_speed, float movement_speed, Entity* target);

    ~Geezer();

    // Override update to include state transitions and attack logic
    void update(float time, float deltaTime) override;

    // Implements the pure virtual control() method.
    // For now, we return NONE since the state logic overrides movement.
    MovementDirection control(float time, float deltaTime) override;

    // Render the enemy and its fireballs
    void render(SDL_Renderer* renderer);

private:
    GeezerState currentState;
    GeezerState prevState;
    SDL_Renderer* renderer;
    Entity* target; // for example, the player

    // destination
    float destinationX;
    float destinationY;

    // Timing for attacks
    float attackInterval;           // seconds between fireballs
    float withdrawAttackInterval;   // attack rate when withdrawing
    float lastAttackTime;           // timestamp of the last fired projectile

    float lastPathfindTime; // last time it pathfinded for strafing
    float sightRange;       // dist > 128       -> idle (can't see)
    float maxAttackRange;   // 80 < dist < 128  -> chase (way too far)
    float idealOuter;       // 72 < dist < 128  -> pursue (a lil too far)
    float idealAttackRange; // where the geezer wants to be
    float idealInner;       // 56 < dist < 72   -> attack (just right)
    float minAttackRange;   // 48 < dist < 56   -> withdraw (a lil too close)
                            // dist < 48        -> flee (way too close)
    
    float projectileSpeed;  // speed of fireballs
                            // should probably be in the fireball class, but eh
    
    std::random_device rd;
    std::mt19937 gen;

    // randomness introduced to fireball shots (std dev in radians)
    // we use 0.045rad or ~1.3deg; means ~98% of shots will be in
    // 5.2deg area centered on target
    float shotVariance;

    // randomness introduced to destination (std dev in radians)
    // we use 0.35rad or ~4.9deg; means ~98% of shots will be in
    // 19.6deg area around the most obvious position
    // this gives the feeling of the geezer strafing around the
    // player
    float posVariance;

    // List of active fireballs fired by this enemy
    std::vector<Fireball*> projectiles;

    // Fire a projectile at the target with some inaccuracy
    void fireAtTarget(float time);

    // Helper: compute Euclidean distance to target
    float distanceToTarget() const;

    // select a destination point close to target
    void setDestination (float time);

    // move to destination while maintaining arc
    void moveToDestination (float deltaTime);
};