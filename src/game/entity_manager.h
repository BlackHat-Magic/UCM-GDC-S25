#pragma once

#include <vector>
#include <memory>
#include <algorithm>
#include <utility> // For std::move, std::forward

#include <SDL2/SDL.h>

#include "entity.h"
#include "player.h"   // Include specific types if needed for helpers
#include "fireball.h" // Include specific types if needed for helpers
#include "utils/tilemap.h"
#include "utils/collisions_defs.h" // Include collision definitions

class EntityManager {
public:
    EntityManager(SDL_Renderer* renderer);
    ~EntityManager() = default; // Default destructor is fine with unique_ptr

    // Template to add any entity type derived from Entity
    template <typename T, typename... Args>
    T* addEntity(Args&&... args) {
        // Ensure the type T is derived from Entity
        static_assert(
            std::is_base_of<Entity, T>::value, "T must derive from Entity"
        );

        // Create unique pointer to new entity
        auto newEntity = std::make_unique<T>(std::forward<Args>(args)...);
        // Get raw pointer before moving ownership (use carefully!)
        T* entityPtr = newEntity.get();
        // Add the unique pointer to the vector
        entities.push_back(std::move(newEntity));
        return entityPtr; // Return raw pointer for convenience
    }

    void update(Tilemap* map, float time, float deltaTime);
    void render();

    void cleanupEntities(); // Remove entities marked for deletion
    void clearAll();        // Remove all entities immediately

    Player* getPlayer() const; // Helper to find the player

    void setScreenDimensions(int width, int height);

private:
    SDL_Renderer* renderer; // Store renderer if needed by entities
    std::vector<std::unique_ptr<Entity>> entities;
    int screenWidth = 640;
    int screenHeight = 480;

    // Collision handling logic
    void handleCollisions();
    // Optional: void handleTileCollisions(Tilemap* map);
};
