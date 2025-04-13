#pragma once
#include <cstdint>

// Define individual layers using single bits
enum class CollisionLayer : uint32_t {
    NONE = 0,

    // --- Basic Physics/Movement Types ---
    GROUND_ENTITY       = 1 << 0,  // Entity that walks on the ground
    AIR_ENTITY          = 1 << 1,  // Entity that flies
    // (reserved)       = 1 << 2,
    // (reserved)       = 1 << 3,

    // --- Interactables ---
    INTERACTABLE        = 1 << 4,  // General interactable object (lever, door?)
    PICKUP              = 1 << 5,  // Item that can be picked up
    // (reserved)       = 1 << 6,
    // (reserved)       = 1 << 7,

    // --- Level Geometry/Hazards ---
    LEVEL_FLOOR         = 1 << 8,  // Non-colliding floor (for reference?)
    LEVEL_WALL          = 1 << 9,  // Standard solid wall
    LEVEL_OBSTACLE      = 1 << 10, // Lower obstacles (ground units collide)
    LEVEL_BOUNDARY      = 1 << 11, // Map edges, room boundaries (all collide)
    LEVEL_PIT           = 1 << 12, // Hole (ground units fall?)
    LEVEL_HAZARD_LAVA   = 1 << 13, // Damaging surface
    LEVEL_HAZARD_TOXIN  = 1 << 14, // Damaging surface
    // (reserved)       = 1 << 15,

    // --- Characters/Projectiles ---
    PLAYER_HITBOX       = 1 << 16, // Player's physical body
    PLAYER_PROJECTILE   = 1 << 17, // Projectile fired by player
    // (reserved)       = 1 << 18,
    // (reserved)       = 1 << 19,
    ENEMY_HITBOX        = 1 << 20, // Enemy's physical body
    ENEMY_PROJECTILE    = 1 << 21, // Projectile fired by enemy
    // (free)           = 1 << 22,
    // (free)           = 1 << 23,

    // --- Free for Use ---
    // (free)           = 1 << 24,
    // (free)           = 1 << 25,
    // (free)           = 1 << 26,
    // (free)           = 1 << 27,
    // (free)           = 1 << 28,
    // (free)           = 1 << 29,
    // (free)           = 1 << 30,
    // (free)           = 1 << 31,

    // --- Common Layer Combinations (What something IS) ---
    LAYER_GROUND_PLAYER = GROUND_ENTITY | PLAYER_HITBOX,
    LAYER_AIR_PLAYER = AIR_ENTITY | PLAYER_HITBOX, // e.g., flight powerup
    LAYER_GROUND_ENEMY = GROUND_ENTITY | ENEMY_HITBOX,
    LAYER_AIR_ENEMY = AIR_ENTITY | ENEMY_HITBOX, // Like the Geezer
    LAYER_PLAYER_PROJECTILE = PLAYER_PROJECTILE, // Projectile is just projectile
    LAYER_ENEMY_PROJECTILE = ENEMY_PROJECTILE,
    LAYER_WALL = LEVEL_WALL | LEVEL_OBSTACLE | LEVEL_BOUNDARY, // General solid tile

    // --- Common Masks (What something COLLIDES WITH) ---
    MASK_GROUND_PLAYER = ENEMY_HITBOX | ENEMY_PROJECTILE | LEVEL_WALL |
                         LEVEL_OBSTACLE | LEVEL_BOUNDARY | LEVEL_PIT |
                         LEVEL_HAZARD_LAVA | LEVEL_HAZARD_TOXIN | PICKUP |
                         INTERACTABLE,
    MASK_AIR_PLAYER = ENEMY_HITBOX | ENEMY_PROJECTILE | LEVEL_WALL |
                      LEVEL_BOUNDARY | PICKUP | INTERACTABLE, // Can fly over obstacles/pits
    MASK_GROUND_ENEMY = PLAYER_HITBOX | PLAYER_PROJECTILE | LEVEL_WALL |
                        LEVEL_OBSTACLE | LEVEL_BOUNDARY | LEVEL_PIT |
                        LEVEL_HAZARD_LAVA | LEVEL_HAZARD_TOXIN | INTERACTABLE,
    MASK_AIR_ENEMY = PLAYER_HITBOX | PLAYER_PROJECTILE | LEVEL_WALL |
                     LEVEL_BOUNDARY | INTERACTABLE, // Can fly over obstacles/pits
    MASK_PLAYER_PROJECTILE = ENEMY_HITBOX | LEVEL_WALL | LEVEL_OBSTACLE |
                             LEVEL_BOUNDARY, // Hits enemies and solid things
    MASK_ENEMY_PROJECTILE = PLAYER_HITBOX | LEVEL_WALL | LEVEL_OBSTACLE |
                            LEVEL_BOUNDARY, // Hits player and solid things
    MASK_PICKUP = PLAYER_HITBOX, // Only player needs to collide to pick up
    MASK_WALL_TILE = GROUND_ENTITY | AIR_ENTITY | PLAYER_PROJECTILE | ENEMY_PROJECTILE // What collides with a basic wall tile
};

// define bitwise OR for CollisionLayer enum
inline CollisionLayer operator|(CollisionLayer lhs, CollisionLayer rhs) {
    return static_cast<CollisionLayer>(
        static_cast<uint32_t>(lhs) | static_cast<uint32_t>(rhs)
    );
}

// define '|=' operator
inline CollisionLayer& operator|=(CollisionLayer& lhs, CollisionLayer rhs) {
    lhs = lhs | rhs;
    return lhs;
}

// define bitwise AND for checking
inline uint32_t operator&(CollisionLayer lhs, CollisionLayer rhs) {
    return static_cast<uint32_t>(lhs) & static_cast<uint32_t>(rhs);
}

// Collision check helper function
inline bool checkCollision(CollisionLayer mask, CollisionLayer layerToCheck) {
    // Check if *any* bit set in layerToCheck is also set in the mask
    return (mask & layerToCheck) != 0;
}
