#pragma once

#include "movement_attack_animated.h"
#include "entity.h"        // Included via movement_attack_animated.h
#include "player.h"        // Needed for target type check potentially
#include "entity_manager.h" // Needed for adding fireballs
#include <vector>
#include <random>
#include <limits> // For numeric_limits

// Define states for the Geezer
enum class GeezerState {
    G_IDLE,
    G_CHASE,
    G_APPROACH,
    G_ATTACK,
    G_WITHDRAW,
    G_FLEE
};

class Geezer : public MovementAttackAnimated {
public:
    Geezer(
        SDL_Renderer* renderer, EntityManager* entityManager,
        const char* sprite_path, int sprite_width, int sprite_height, float x,
        float y, int** animations, float animation_speed,
        float movement_speed, Entity* target
    );

    // Control now modifies vx, vy directly
    void control(Tilemap* map, float time, float deltaTime) override;

private:
    EntityManager* entityManager; // To spawn fireballs
    GeezerState currentState;
    GeezerState prevState;
    SDL_Renderer* renderer; // Needed to pass to Fireball constructor
    Entity* target;         // The entity the Geezer targets (e.g., player)

    // Destination for movement states
    float destinationX = 0.0f;
    float destinationY = 0.0f;

    // Timing for attacks
    float attackInterval; // seconds between fireballs
    float lastAttackTime; // timestamp of the last fired projectile

    // AI parameters
    float lastPathfindTime; // last time it chose a new destination
    float sightRange;       // Max distance to see target
    float maxAttackRange;   // Outer range for chasing
    float idealOuter;       // Outer range for approaching
    float idealAttackRange; // Target distance for attacking/strafing
    float idealInner;       // Inner range for withdrawing
    float minAttackRange;   // Inner range for fleeing

    float projectileSpeed; // Speed of fireballs

    // Randomness for AI behavior
    std::random_device rd;
    std::mt19937 gen;
    float shotVariance; // Inaccuracy for shots (std dev in radians)
    float posVariance;  // Randomness for destination selection (std dev radians)

    // AI Helper methods
    void fireAtTarget(float time);
    float distanceToTarget() const;
    void setDestination(float time); // Calculate a new movement destination
    void moveToDestination();        // Set vx, vy towards current destination
};
