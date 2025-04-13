#include <cmath>
#include <cstdlib>
#include <algorithm>
#include <iostream> // For debugging

#include "geezer.h"
#include "fireball.h" // Include fireball to spawn it

Geezer::Geezer(
    SDL_Renderer* renderer, EntityManager* entityManager,
    const char* sprite_path, int sprite_width, int sprite_height, float x,
    float y, int** animations, float animation_speed, float movement_speed,
    Entity* target
) :
    MovementAttackAnimated(
        renderer, sprite_path, sprite_width, sprite_height, x, y, animations,
        animation_speed, movement_speed
    ),
    entityManager(entityManager),
    currentState(GeezerState::G_IDLE),
    prevState(GeezerState::G_IDLE),
    renderer(renderer), // Store renderer for fireball creation
    target(target),
    attackInterval(1.0f),
    lastAttackTime(0.0f),
    lastPathfindTime(0.0f),
    sightRange(256.0f),
    maxAttackRange(128.0f),
    idealOuter(112.0f),
    idealAttackRange(96.0f),
    idealInner(80.0f),
    minAttackRange(64.0f),
    projectileSpeed(300.0f),
    shotVariance(0.045f), // Approx +/- 2.6 degrees std dev
    posVariance(0.35f),   // Approx +/- 20 degrees std dev
    gen(rd()) {
    // Set initial animation (assuming animations are provided correctly)
    setAnimation(0);
    setStage(0);

    // Set collision layer and mask
    layer = CollisionLayer::LAYER_AIR_ENEMY; // Geezer flies
    mask = CollisionLayer::MASK_AIR_ENEMY;
}

void Geezer::control(Tilemap* map, float time, float deltaTime) {
    // Reset velocity each frame
    vx = 0.0f;
    vy = 0.0f;

    if (!target || target->isMarkedForDeletion()) {
        currentState = GeezerState::G_IDLE;
        // No movement if no target or target gone
        return;
    }

    // --- State Logic ---
    float dist = distanceToTarget();
    prevState = currentState;

    // Determine state based on distance
    if (dist > sightRange) {
        currentState = GeezerState::G_IDLE;
    } else if (dist > maxAttackRange) {
        currentState = GeezerState::G_CHASE;
    } else if (dist > idealOuter) {
        currentState = GeezerState::G_APPROACH;
    } else if (dist > idealInner) {
        currentState = GeezerState::G_ATTACK;
    } else if (dist > minAttackRange) {
        currentState = GeezerState::G_WITHDRAW;
    } else {
        currentState = GeezerState::G_FLEE;
    }

    // --- Destination Logic ---
    // Update destination if state changed or periodically, or if target moved significantly
    bool targetMovedSignificantly = false;
    if (currentState != GeezerState::G_IDLE && currentState != GeezerState::G_ATTACK) {
         SDL_Point pt = target->getPosition();
         float targetX = static_cast<float>(pt.x);
         float targetY = static_cast<float>(pt.y);
         float destDx = targetX - destinationX;
         float destDy = targetY - destinationY;
         // Check if current destination is now outside the desired range relative to target's *current* pos
         float destDistToTarget = std::sqrt(destDx * destDx + destDy * destDy);
         // Example threshold: if destination is > 32 pixels off from ideal range center
         if (std::abs(destDistToTarget - idealAttackRange) > 32.0f) {
              targetMovedSignificantly = true;
         }
    }


    if (prevState != currentState || targetMovedSignificantly || (time - lastPathfindTime > 2.0f)) {
        setDestination(time);
    }


    // --- Action Logic ---
    // Move towards destination if in a moving state
    if (currentState != GeezerState::G_IDLE && currentState != GeezerState::G_ATTACK) {
        moveToDestination(); // Sets vx, vy
    }

    // Fire projectile based on state and timing
    fireAtTarget(time);

    // Base class update will handle animation and actual movement/collision
}

void Geezer::fireAtTarget(float time) {
    // Check if allowed to fire based on state
    bool canFire = false;
    float currentInterval = attackInterval;

    switch (currentState) {
    case GeezerState::G_CHASE:
        currentInterval = 2.0f * attackInterval; // Fire less often when chasing
        canFire = true;
        break;
    case GeezerState::G_APPROACH:
    case GeezerState::G_ATTACK:
        canFire = true; // Normal firing rate
        break;
    case GeezerState::G_WITHDRAW:
        currentInterval = attackInterval / 2.0f; // Fire more often when withdrawing
        canFire = true;
        break;
    case GeezerState::G_IDLE:
    case GeezerState::G_FLEE:
    default:
        canFire = false; // Don't fire when idle or fleeing
        break;
    }

    // Check timing
    if (!canFire || (time - lastAttackTime < currentInterval)) {
        return;
    }

    // --- Fire the projectile ---
    if (!target) return; // Should not happen if state logic is correct, but safety check

    SDL_Point tgtPos = target->getPosition();
    float targetX = static_cast<float>(tgtPos.x);
    float targetY = static_cast<float>(tgtPos.y);

    // Calculate base angle to target
    float dx = targetX - x;
    float dy = targetY - y;
    float baseAngle = std::atan2(dy, dx);

    // Add inaccuracy based on state
    float currentShotVariance = shotVariance;
    if (currentState == GeezerState::G_WITHDRAW || currentState == GeezerState::G_FLEE) {
        currentShotVariance *= 2.0f; // Less accurate when retreating/panicked
    }
    std::normal_distribution<float> dist(baseAngle, currentShotVariance);
    float randomAngle = dist(gen);

    // Calculate projectile velocity vector
    float proj_vx = std::cos(randomAngle) * projectileSpeed;
    float proj_vy = std::sin(randomAngle) * projectileSpeed;

    // Spawn the fireball using EntityManager
    // Assuming fireball sprite is 16x16
    entityManager->addEntity<Fireball>(
        renderer,                      // Pass the stored renderer
        "assets/sprites/fireball.png", // Sprite path
        16, 16,                        // Sprite dimensions
        x, y,                          // Initial position (Geezer's position)
        proj_vx, proj_vy,              // Initial velocity
        this,                          // Owner is this Geezer instance
        10.0f                          // Damage amount
    );

    lastAttackTime = time; // Record the time of the shot
    attack(time);          // Trigger the attack animation in the base class
}

void Geezer::moveToDestination() {
    // vx, vy are already reset in control()
    if (currentState == GeezerState::G_IDLE || currentState == GeezerState::G_ATTACK) {
        return; // Don't move in these states
    }

    float dx = destinationX - x;
    float dy = destinationY - y;
    float distSq = dx * dx + dy * dy;

    // Stop if very close to the destination
    float thresholdSq = 4.0f; // Stop within 2 pixels
    if (distSq < thresholdSq) {
        return; // Already at destination
    }

    // Normalize direction vector and scale by movement speed
    float dist = std::sqrt(distSq);
    if (dist > 0) { // Avoid division by zero
        vx = (dx / dist) * movementSpeed;
        vy = (dy / dist) * movementSpeed;
    }
}

float Geezer::distanceToTarget() const {
    if (!target) return std::numeric_limits<float>::max();

    SDL_Point pt = target->getPosition();
    float dx = static_cast<float>(pt.x) - x;
    float dy = static_cast<float>(pt.y) - y;
    return std::sqrt(dx * dx + dy * dy);
}

void Geezer::setDestination(float time) {
    if (currentState == GeezerState::G_IDLE || currentState == GeezerState::G_ATTACK) {
        destinationX = x; // Stay put
        destinationY = y;
        lastPathfindTime = time;
        return;
    }

    if (!target) return; // No target, no destination

    SDL_Point pt = target->getPosition();
    float targetX = static_cast<float>(pt.x);
    float targetY = static_cast<float>(pt.y);

    // Base angle from target towards Geezer (for orbiting)
    float angleToGeezer = std::atan2(y - targetY, x - targetX);

    // Determine target distance based on state
    float targetDist = idealAttackRange;
    if (currentState == GeezerState::G_CHASE || currentState == GeezerState::G_APPROACH) {
         targetDist = idealOuter; // Try to get to the edge of ideal range
    } else if (currentState == GeezerState::G_WITHDRAW || currentState == GeezerState::G_FLEE) {
         targetDist = idealInner; // Try to back off to the inner edge
    }


    // Add randomness for strafing effect
    std::normal_distribution<float> dist(angleToGeezer, posVariance);
    float randomAngle = dist(gen);

    // Calculate destination point around the target
    destinationX = targetX + std::cos(randomAngle) * targetDist;
    destinationY = targetY + std::sin(randomAngle) * targetDist;

    lastPathfindTime = time; // Record when destination was set
}
