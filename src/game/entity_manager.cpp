#include "entity_manager.h"

EntityManager::EntityManager (SDL_Renderer* renderer) : renderer (renderer) {}

void EntityManager::setScreenDimensions (int width, int height) {
    screenWidth = width;
    screenHeight = height;
}

void EntityManager::update (Tilemap* map, float time, float deltaTime) {
    for (auto& entity : entities) {
        if (entity && !entity->isMarkedForDeletion ()) {
            entity->update(map, time, deltaTime);
        }
    }

    handleCollisions ();

    for (auto& entity : entities) {
        if (!entity || entity->isMarkedForDeletion ()) {
            continue;
        }
        auto* fb = dynamic_cast<Fireball*>(entity.get());
        if(!fb) {
            continue;
        }
        // mark off-screen fireballs for deletion
        // maybe extend to other entities in the future
        if (fb->isOffScreen (screenWidth, screenHeight)) {
            fb->markForDeletion ();
        }
    }
}

void EntityManager::render () {
    for (const auto& entity : entities) {
        if (entity) {
            entity->render (renderer);
        }
    }
}

void EntityManager::cleanupEntities () {
    // remove entities marked for deletion
    entities.erase (std::remove_if (
        entities.begin (), 
        entities.end (), 
        [](const std::unique_ptr<Entity>& entity) {
            return !entity || entity->isMarkedForDeletion ();
        }
    ), entities.end ());
}

void EntityManager::clearAll () {
    entities.clear ();
}

// helper function to get the player specifically.
Player* EntityManager::getPlayer() const {
    for (const auto& entity : entities) {
        if (Player* player = dynamic_cast<Player*>(entity.get ())) {
            return (player);
        }
    }
    return nullptr;
}

void EntityManager::handleCollisions () {
    for (size_t i = 0; i < entities.size (); i++) {
        Entity* entityA = entities[i].get ();

        // skip nonexistent/inactive
        if (!entityA || entityA->isMarkedForDeletion ()) continue;

        // see what it collides with
        for (size_t j = i + 1; j < entities.size(); ++j) {
            Entity* entityB = entities[j].get();
            if (!entityB || entityB->isMarkedForDeletion()) continue;

            // --- Layer/Mask Check ---
            if (!checkCollision(entityA->mask, entityB->layer) &&
                !checkCollision(entityB->mask, entityA->layer)) {
                continue; // These entities don't interact
            }

            // --- Narrowphase Collision Check (e.g., AABB) ---
            SDL_Rect rectA = { /* get rect for A */ };
            SDL_Rect rectB = { /* get rect for B */ };

            if (SDL_HasIntersection(&rectA, &rectB)) {
                // --- Handle Collision Response ---
                // This is where specific logic goes (damage, effects, etc.)
                // Example: Player vs Enemy Fireball
                Player* player = dynamic_cast<Player*>(entityA);
                Fireball* fb = dynamic_cast<Fireball*>(entityB);
                if (player && fb && fb->getOwner() != player) {
                     player->takeDamage(fb->getDamage());
                     fb->markForDeletion();
                }
                // Handle the other way around (A=Fireball, B=Player)
                player = dynamic_cast<Player*>(entityB);
                fb = dynamic_cast<Fireball*>(entityA);
                 if (player && fb && fb->getOwner() != player) {
                     player->takeDamage(fb->getDamage());
                     fb->markForDeletion();
                }
                // Add other interaction types (Enemy vs Player, etc.)
            }
        }
    }
}