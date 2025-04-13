#pragma once

#include <vector>
#include <memory>
#include <algorithm>
#include <utility>

#include <SDL2/SDL.h>

#include "entity.h"
#include "player.h"
#include "fireball.h"
#include "utils/tilemap.h"

// class Fireball;

class EntityManager {
public:
    EntityManager (SDL_Renderer* renderer);
    ~EntityManager () = default;

    // template to add any entity type derived from entity
    template <typename T, typename... Args>
    T* addEntity (Args&&... args) {
        // create unique pointer to new entity
        auto newEntity = std::make_unique<T>(std::forward<Args>(args)...);
        // get raw pointer before moving ownership
        T* entityPtr = newEntity.get ();
        // add the unique pointer to the vector
        entities.push_back (std::move(newEntity));
        return entityPtr;
    }

    void update (Tilemap* map, float time, float deltaTime);

    void render ();

    // remove all entities (e.g., quitting, main menu)
    void cleanupEntities ();

    void clearAll ();

    Player* getPlayer () const;

    void setScreenDimensions (int width, int height);

private:
    SDL_Renderer* renderer;
    std::vector<std::unique_ptr<Entity>> entities;
    int screenWidth  = 640;
    int screenHeight = 480;

    void handleCollisions ();
};