#include <cmath>
#include <cstdlib>
#include <algorithm>
#include <iostream>

#include "geezer.h"

Geezer::Geezer(SDL_Renderer* renderer, EntityManager* entityManager,
            const char* sprite_path, int sprite_width, int sprite_height, float x,
            float y, int** animations, float animation_speed, float movement_speed,
            Entity* target)
    : MovementAttackAnimated(renderer, sprite_path, sprite_width, sprite_height,
                x, y, animations, animation_speed, movement_speed),
        entityManager(entityManager),
        currentState(G_IDLE),
        prevState(G_IDLE),
        renderer(renderer),
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
        
        shotVariance(0.045f),
        posVariance(0.35f),
        
        gen(rd()) {
    // Set an initial animation if needed.
    setAnimation(0);
    setStage(0);
}

void Geezer::control(Tilemap *map, float time, float deltaTime) {
    if (!target || target->isMarkedForDeletion ()) {
        currentState = G_IDLE;
        return;
    }

    // reset each frame
    vx = 0.0f;
    vy = 0.0f;

    // Decide state based on distance to target
    float dist = distanceToTarget();

    // set current state
    prevState = currentState;
    if (dist > sightRange) {
        currentState = G_IDLE;
    } else if (dist > maxAttackRange) {
        currentState = G_CHASE;
    } else if (dist > idealOuter) {
        currentState = G_APPROACH;
    } else if (dist > idealInner) {
        currentState = G_ATTACK;
    } else if (dist > minAttackRange) {
        currentState = G_WITHDRAW;
    } else {
        currentState = G_FLEE;
    }
    // if state has changed, we have a new target destination
    if (prevState != currentState) {
        setDestination(time);
    } else if (time - lastPathfindTime > 2.0f) {
        // new position if we haven't moved in >2sec
        setDestination(time);
    }
    
    // check AGAIN if the player is too far
    // because for some god-damned reason it sometimes just doesn't
    // set a new destination
    // but for *some reason*, this is reliable

    // wait I understand now. It's possible for the player to move
    // to a new position that necessitates a new destination
    // without the Geezer changing state in under 2 seconds
    // which results in the Geezer having an outdated destination
    SDL_Point pt = target->getPosition();
    float targetX = static_cast<float>(pt.x);
    float targetY = static_cast<float>(pt.y);
    float destdx = targetX - destinationX;
    float destdy = targetY - destinationY;
    float destDistToTarget = std::sqrt(destdx*destdx + destdy*destdy);
    if (destDistToTarget > maxAttackRange || destDistToTarget < minAttackRange) {
        setDestination (time);
    }

    // if we're in a moving state, move toward destination
    moveToDestination();

    // if we can fire, fire
    fireAtTarget(time);

    return dir;
}

// handles state-specific firing logic
void Geezer::fireAtTarget(float time) {
    if (currentState == G_IDLE || currentState == G_FLEE) {
        // idle and flee don't fire
        return;
    } else if (currentState == G_CHASE) {
        // chase fire infrequently
        if (time - lastAttackTime < 2.0f * attackInterval) {
            return;
        }
    } else if (currentState == G_APPROACH || currentState == G_ATTACK) {
        // pursue and attack fire as normal
        if (time - lastAttackTime < attackInterval) {
            return;
        }
    } else if (currentState == G_WITHDRAW) {
        // withdraw fires extra frequently
        if (time - lastAttackTime < attackInterval / 2.0f) {
            return;
        }
    }
    
    // Get the target's location.
    SDL_Point tgt = target->getPosition();

    // Compute the desired firing angle (in radians)
    float dx = static_cast<float>(tgt.x) - x;
    float dy = static_cast<float>(tgt.y) - y;
    float baseAngle = std::atan2(dy, dx);

    // Introduce randomness: e.g. up to +/-5deg (approx)
    float currentShotVariance = shotVariance;
    if (currentState == G_WITHDRAW) {
        // if it's withdrawing, it's panicked and is less accurate
        currentShotVariance *= 2.0f;
    }
    std::normal_distribution<float> d{baseAngle, currentShotVariance};
    float randomAngle = d(gen);

    // Determine projectile velocity vector
    float vx = std::cos(randomAngle) * projectileSpeed;
    float vy = std::sin(randomAngle) * projectileSpeed;

    // Create a new fireball in entity manager
    entityManager->addEntity<Fireball>(
        renderer,
        "assets/sprites/fireball.png",
        16, 16,
        x, y,
        vx, vy,
        this,
        10.0f
    );

    lastAttackTime = time;
    attack(time);
}

// handles state-specific speeds
void Geezer::moveToDestination() {
    // don't move if idle or already attacking
    if (currentState == G_IDLE || currentState == G_ATTACK) {
        return;
    }
    // vx, vy already reset for us
    
    // displacement to target position
    float dx = destinationX - x;
    float dy = destinationY - y;
    float mag = dx*dx + dy*dy;

    float threshold = 1.0f;
    if (mag < threshold) {
        return;
    }

    vx = dx / mag * movementSpeed;
    vy = dx / mag * movementSpeed;

    // in the future, we want to avoid getting too close to the player
    // e.g., we're trying to move from one side of circle around player to
    // another, and that line takes us too close to the player
    // ideally, we would instead follow an arc around the player
    // but this is kinda a distraction, so worry about it later
}

float Geezer::distanceToTarget() const {
    // float_max if no target
    if (!target) return std::numeric_limits<float>::max();

    // otherwise self-explanatory
    SDL_Point pt = target->getPosition();
    float dx = static_cast<float>(pt.x) - x;
    float dy = static_cast<float>(pt.y) - y;
    return std::sqrt(dx * dx + dy * dy);
}

void Geezer::setDestination (float time) {
    // if we're supposed to stand still; don't bother
    if (currentState == G_IDLE || currentState == G_ATTACK) {
        destinationX = x;
        destinationY = y;
        return;
    }
    if (!target) {
        return;
    }

    // get target position
    SDL_Point pt = target->getPosition();
    float ptx = static_cast<float>(pt.x);
    float pty = static_cast<float>(pt.y);

    // angle from target to geezer
    float baseAngle = std::atan2(y - pty, x - ptx);

    // Introduce randomness: e.g. up to +/-10deg (approx)
    std::normal_distribution<float> d{baseAngle, posVariance};
    float randomAngle = d(gen);

    // set destination x and destination y
    destinationX = ptx + std::cos(randomAngle) * idealAttackRange;
    destinationY = pty + std::sin(randomAngle) * idealAttackRange;

    // set last pathfind time
    lastPathfindTime = time;
}