#include "entity_manager.h"
#include <iostream> // For debugging

EntityManager::EntityManager(SDL_Renderer* renderer) : renderer(renderer) {}

void EntityManager::setScreenDimensions(int width, int height) {
    screenWidth = width;
    screenHeight = height;
}

void EntityManager::update(Tilemap* map, float time, float deltaTime) {
    // 1. Update all active entities (handles movement, AI, animation)
    for (auto& entity : entities) {
        if (entity && !entity->isMarkedForDeletion()) {
            entity->update(map, time, deltaTime); // Pass map for tile collisions
        }
    }

    // 2. Handle entity-entity collisions
    handleCollisions();

    // 3. Post-update checks (e.g., off-screen removal)
    for (auto& entity : entities) {
        if (!entity || entity->isMarkedForDeletion()) {
            continue;
        }

        // Example: Remove off-screen fireballs
        if (auto* fb = dynamic_cast<Fireball*>(entity.get())) {
            if (fb->isOffScreen(screenWidth, screenHeight)) {
                fb->markForDeletion();
            }
        }
        // Add checks for other entity types if needed
    }

    // 4. Clean up entities marked for deletion (done at end of frame or start of next)
    // cleanupEntities(); // Moved to main loop or called explicitly when needed
}

void EntityManager::render() {
    // Simple render loop
    for (const auto& entity : entities) {
        if (entity && !entity->isMarkedForDeletion()) { // Optionally render dying entities?
            entity->render(renderer);
        }
    }
}

void EntityManager::cleanupEntities() {
    // Remove entities marked for deletion using erase-remove idiom
    entities.erase(
        std::remove_if(
            entities.begin(), entities.end(),
            [](const std::unique_ptr<Entity>& entity) {
                // Remove if pointer is null or entity is marked
                return !entity || entity->isMarkedForDeletion();
            }
        ),
        entities.end()
    );
}

void EntityManager::clearAll() {
    entities.clear(); // Destructors of unique_ptr will handle cleanup
}

Player* EntityManager::getPlayer() const {
    for (const auto& entity : entities) {
        // Check if entity exists and is a Player
        if (entity) {
            if (Player* player = dynamic_cast<Player*>(entity.get())) {
                // Optionally check if player is alive?
                // if (player->isAlive()) {
                return player;
                // }
            }
        }
    }
    return nullptr; // No player found (or player is not alive)
}

void EntityManager::handleCollisions() {
    // Simple N^2 collision check (optimize with broadphase later if needed)
    for (size_t i = 0; i < entities.size(); ++i) {
        Entity* entityA = entities[i].get();
        if (!entityA || entityA->isMarkedForDeletion()) continue;

        for (size_t j = i + 1; j < entities.size(); ++j) {
            Entity* entityB = entities[j].get();
            if (!entityB || entityB->isMarkedForDeletion()) continue;

            // --- Layer/Mask Check ---
            // Check if A cares about B OR B cares about A
            bool checkA = checkCollision(entityA->mask, entityB->layer);
            bool checkB = checkCollision(entityB->mask, entityA->layer);

            if (!checkA && !checkB) {
                continue; // These entities don't interact based on layers/masks
            }

            // --- Narrowphase Collision Check (AABB using SDL_FRect) ---
            SDL_FRect rectA = entityA->getBoundingBox();
            SDL_FRect rectB = entityB->getBoundingBox();

            if (SDL_HasIntersectionF(&rectA, &rectB)) {
                // --- Handle Collision Response ---
                // This part needs specific logic for each interaction type.
                // Using dynamic_cast is one way, but can get complex.
                // Consider components or a message system for larger projects.

                // Example: Player vs Enemy Fireball
                Player* player = nullptr;
                Fireball* fb = nullptr;

                // Case 1: A is Player, B is Fireball
                if ((player = dynamic_cast<Player*>(entityA)) && (fb = dynamic_cast<Fireball*>(entityB))) {
                     // Check if it's an ENEMY fireball hitting the player
                     if (checkCollision(player->mask, fb->layer) && fb->getOwner() != player) {
                          player->takeDamage(fb->getDamage());
                          fb->markForDeletion();
                          continue; // Collision handled for this pair
                     }
                }

                // Case 2: A is Fireball, B is Player
                if ((fb = dynamic_cast<Fireball*>(entityA)) && (player = dynamic_cast<Player*>(entityB))) {
                     // Check if it's an ENEMY fireball hitting the player
                     if (checkCollision(player->mask, fb->layer) && fb->getOwner() != player) {
                          player->takeDamage(fb->getDamage());
                          fb->markForDeletion();
                          continue; // Collision handled for this pair
                     }
                }

                // Example: Enemy vs Player Projectile (add similar logic)
                // ... check for Enemy* and Fireball* where owner is Player ...

                // Example: Player vs Enemy Hitbox (add similar logic)
                // ... check for Player* and Enemy* (e.g. Geezer*) ...
                // ... apply damage or other effects ...

            }
        }
    }
}
