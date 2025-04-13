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
    Player* player = getPlayer ();
    if (!player || !player->isAlive ()) {
        // physics don't really matter if the player is dead
        return;
    }

    SDL_Rect playerRect = {
        static_cast<int>(player->x),
        static_cast<int>(player->y),
        player->spriteWidth,
        player->spriteHeight,
    };

    for (auto& entity: entities) {
        // only check active fireballs
        if (!entity || entity->isMarkedForDeletion ()) {
            continue;
        }
        // again, only active fireballs
        // extend to others in the future
        Fireball* fb = dynamic_cast<Fireball*>(entity.get());
        if(!fb) {
            continue;
        }
        // ignore fireballs owned by the player for now
        if (fb->getOwner () == player) {
            continue;
        }
        SDL_Rect fireballRect = {
            static_cast<int>(fb->x),
            static_cast<int>(fb->y),
            fb->spriteWidth,
            fb->spriteHeight
        };
        if(SDL_HasIntersection(&fireballRect, &playerRect)) {
            player->takeDamage (fb->getDamage ());
            fb->markForDeletion ();
        }
    }
}