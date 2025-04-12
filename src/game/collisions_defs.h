#pragma once
#include <cstdint>

enum class CollisionLayer : uint32_t {
    NONE = 0;

    // basic movement types
    GROUND              = 0b0000'0000'0000'0000'0000'0000'0000'0001;
    // (reserved)       = 0b0000'0000'0000'0000'0000'0000'0000'0010;
    // (reserved)       = 0b0000'0000'0000'0000'0000'0000'0000'0100;
    // (reserved)       = 0b0000'0000'0000'0000'0000'0000'0000'1000;
    AIR                 = 0b0000'0000'0000'0000'0000'0000'0001'0000;
    // (reserved)       = 0b0000'0000'0000'0000'0000'0000'0010'0000;
    // (reserved)       = 0b0000'0000'0000'0000'0000'0000'0100'0000;
    // (reserved)       = 0b0000'0000'0000'0000'0000'0000'1000'0000;

    // level geometry/hazards
    LEVEL_FLOOR         = 0b0000'0000'0000'0000'0000'0001'0000'0000;
    LEVEL_WALL          = 0b0000'0000'0000'0000'0000'0010'0000'0000;
    LEVEL_OBSTACLE      = 0b0000'0000'0000'0000'0000'0100'0000'0000;
    LEVEL_BOUNDARY      = 0b0000'0000'0000'0000'0000'1000'0000'0000;
    LEVEL_PIT           = 0b0000'0000'0000'0000'0001'0000'0000'0000;
    LEVEL_HAZARD_LAVA   = 0b0000'0000'0000'0000'0010'0000'0000'0000;
    LEVEL_HAZARD_TOXIN  = 0b0000'0000'0000'0000'0100'0000'0000'0000;
    // (reserved)       = 0b0000'0000'0000'0000'1000'0000'0000'0000;

    // interactables
    INTERACTABLE        = 0b0000'0000'0000'0001'0000'0000'0000'0000; // e.g., chest
    PICKUP              = 0b0000'0000'0000'0010'0000'0000'0000'0000; // powerups
    // (reserved)       = 0b0000'0000'0000'0100'0000'0000'0000'0000;
    // (reserved)       = 0b0000'0000'0000'1000'0000'0000'0000'0000;

    // (free)           = 0b0000'0000'0001'0000'0000'0000'0000'0000;
    // (free)           = 0b0000'0000'0010'0000'0000'0000'0000'0000;
    // (free)           = 0b0000'0000'0100'0000'0000'0000'0000'0000;
    // (free)           = 0b0000'0000'1000'0000'0000'0000'0000'0000;

    // characters
    PLAYER_HITBOX       = 0b0000'0001'0000'0000'0000'0000'0000'0000;
    PLAYER_PROJECTILE   = 0b0000'0010'0000'0000'0000'0000'0000'0000;
    // (reserved)       = 0b0000'0100'0000'0000'0000'0000'0000'0000;
    // (reserved)       = 0b0000'1000'0000'0000'0000'0000'0000'0000;
    ENEMY_HITBOX        = 0b0001'0000'0000'0000'0000'0000'0000'0000;
    PLAYER_PROJECTILE   = 0b0010'0000'0000'0000'0000'0000'0000'0000;
    // (reserved)       = 0b0100'0000'0000'0000'0000'0000'0000'0000;
    // (reserved)       = 0b1000'0000'0000'0000'0000'0000'0000'0000;

    // common stuff for characters
    GROUND_PLAYER = GROUND | PLAYER_HITBOX,
    AIR_PLAYER = AIR | PLAYER_HITBOX, // e.g., player has a flight powerup
    PLAYER_PROJECTILE = LEVEL_WALL | LEVEL_OBSTACLE | LEVEL_BOUNDARY | \
        INTERACTABLE | PICKUP | ENEMY_HITBOX,
    
    GROUND_ENEMY = GROUND | ENEMY_HITBOX,
    AIR_ENEMY = AIT | ENEMY_HITBOX,
    ENEMY_PROJECTILE = LEVEL_WALL | LEVEL_OBSTACLE | LEVEL_BOUNDARY | \
        INTERACTABLE | PICKUP | ENEMY_HITBOX,

    // common character masks
    GROUND_PLAYER_MASK = ENEMY | ENEMY_PROJECTILE | LEVEL_WALL | LEVEL_OBSTACLE | \
        LEVEL_BOUNDARY | LEVEL_PIT | LEVEL_HAZARD_LAVA | LEVEL_HAZARD_TOXIN | \
        PICKUP | INTERACTABLE,
    AIR_PLAYER_MASK = ENEMY | ENEMY_PROJECTILE | LEVEL_WALL | LEVEL_BOUNDARY | \
        PICKUP | INTERACTABLE,
    GROUND_ENEMY_MASK = PLAYER | PLAYER_PROJECTILE | LEVEL_WALL | LEVEL_OBSTACLE |\
        LEVEL_BOUNDARY | LEVEL_PIT | LEVEL_HAZARD_LAVA | LEVEL_HAZARD_TOXIN,
    AIR_ENEMY_MASK = PLAYER | PLAYER_PROJECTILE | LEVEL_WALL | LEVEL_BOUNDARY | \
        INTERACTABLE,

    // common projectile masks
    PLAYER_PROJECTILE_MASK = ENEMY | LEVEL_WALL | LEVEL_OBSTACLE | LEVEL_BOUNDARY,
    ENEMY_PROJECTILE_MASK = PLAYER | LEVEL_WALL | LEVEL_OBSTACLE | LEVEL_BOUNDARY,

    // pickup mask
    PICKUP_MASK = PLAYER | LEVEL_WALL | LEVEL_OBSTACLE | LEVEL_BOUNDARY,
}

// Collision check helper functions
inline bool checkCollision (uint32_t mask, uint32_t layerToCheck) {
    return (mask & layerToCheck) != 0;
}
inline bool checkCollision (CollisionLayer mask, CollisionLayer layerToCheck) {
    return (static_cast<uint32_t)(mask) & static_cast<uint32_t>(layerToCheck) != 0;
}

// define bitwise OR for CollisionLayer enum
inline CollisionLayer operator|(CollisionLayer lhs, CollisionLayer rhs) {
    return static_cast<CollisionLayer> {
        static_cast<uint32_t>(lhs) | static_cast<uint32_t>(rhs)
    }
}

// define '|=' operator
inline CollisionLayer& operator|=(CollisionLayer& lhs, CollisionLayer rhs) {
    lhs = lhs | rhs;
    return lhs;
}