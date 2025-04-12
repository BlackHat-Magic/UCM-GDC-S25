#include "geezer.h"
#include <cmath>
#include <cstdlib>

Geezer::Geezer(SDL_Renderer* renderer, const char* sprite_path, int sprite_width,
               int sprite_height, float x, float y, int** animations,
               float animation_speed, float movement_speed, Entity* target)
    : MovementAttackAnimated(renderer, sprite_path, sprite_width, sprite_height,
                x, y, animations, animation_speed, movement_speed),
        currentState(G_IDLE),
        prevState(G_IDLE),
        renderer(renderer),
        target(target),
        attackInterval(1.0f),
        withdrawAttackInterval(2.0f),
        lastAttackTime(0.0f),
        lastPathfindTime(0.0f),
        sightRange(256.0f),
        maxAttackRange(80.0f),
        idealOuter(72.0f),
        idealAttackRange(64.0f),
        idealInner(56.0f),
        minAttackRange(48.0f),
        projectileSpeed(300.0f),
        shotVariance(0.045f),
        posVariance(0.35f),
        gen(rd()) {
    // Set an initial animation if needed.
    setAnimation(0);
    setStage(0);
}

void Geezer::update(float time, float deltaTime) {
    // Decide state based on distance to target
    float dist = distanceToTarget();
    
    // update previous state
    prevState = currentState;

    // set current state
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

    // if we're in a moving state, move toward destination
    moveToDestination(deltaTime);

    // if we can fire, fire
    fireAtTarget(time);

    // Update animation (provided by MovementAttackAnimated)
    advanceAnimation();

    // Update projectiles.
    for (auto it = projectiles.begin(); it != projectiles.end();) {
        (*it)->update(time, deltaTime);
        ++it;
    }
}

MovementDirection Geezer::control(float time, float deltaTime) {
    // Return NONE so that MovementAttackAnimated does not override
    return NONE;
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
    } else if (currentState == G_CHASE || currentState == G_ATTACK) {
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

    // Create a new fireball.
    // Assuming we have a fireball spritesheet with a suitable sprite.
    Fireball* fb = new Fireball(
        renderer, 
        "assets/sprites/fireball.png", 
        16, 16, 
        x, y, 
        vx, vy, 
        this, 
        10.0f
    );
    projectiles.push_back(fb);

    lastAttackTime = time;
}

// handles state-specific speeds
void Geezer::moveToDestination (float deltaTime) {
    // don't move if idle or already attacking
    if (currentState == G_IDLE || currentState == G_ATTACK) {
        return;
    }
    
    // displacement to target position
    float dx = destinationX - x;
    float dy = destinationY - y;
    float mag = std::sqrt(dx*dx + dy*dy);
    if (mag <= 0.0f) {
        // already at destination
        return;
    } else {
        dx /= mag;
        dy /= mag;
    }

    x += dx * movementSpeed * deltaTime;
    y += dy * movementSpeed * deltaTime;
    return;

    // in the future, we want to avoid getting too close to the player
    // e.g., we're trying to move from one side of circle around player to
    // another, and that line takes us too close to the player
    // ideally, we would instead follow an arc around the player
    // but this is kinda a distraction, so worry about it later
}

void Geezer::render(SDL_Renderer* renderer) {
    // Render the geezer itself
    MovementAttackAnimated::render(renderer);

    // Render each active fireball.
    for (auto fb : projectiles)
        fb->render(renderer);
}

float Geezer::distanceToTarget() const {
    SDL_Point pt = target->getPosition();
    float dx = static_cast<float>(pt.x) - x;
    float dy = static_cast<float>(pt.y) - y;
    return std::sqrt(dx * dx + dy * dy);
}

void Geezer::setDestination (float time) {
    SDL_Point pt = target->getPosition();
    float ptx = static_cast<float>(pt.x);
    float pty = static_cast<float>(pt.y);
    
    // displacement from target to geezer
    float dx = x - ptx;
    float dy = y - pty;

    // angle from target to geezer
    float baseAngle = std::atan2(dy, dx);

    // Introduce randomness: e.g. up to +/-10deg (approx)
    std::normal_distribution<float> d{baseAngle, posVariance};
    float randomAngle = d(gen);

    // set destination x and destination y
    destinationX = ptx + std::cos(randomAngle) * idealAttackRange;
    destinationY = pty + std::sin(randomAngle) * idealAttackRange;

    // set last pathfind time
    lastPathfindTime = time;
}