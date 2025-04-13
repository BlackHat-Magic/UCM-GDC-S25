#pragma once
#include <SDL2/SDL.h>
#include <vector>
#include <map> // For mapping tile index to layer
#include "spritesheet.h"
#include "collisions_defs.h" // Include collision definitions
#include "direction.h"       // Keep for now if needed elsewhere

class Tilemap {
public:
    // Constructor now takes a map defining which tile indices map to which collision layers
    Tilemap(
        Spritesheet* sheet, int tile_width, int tile_height, int map_width,
        int map_height, const std::map<int, CollisionLayer>& tile_collision_layers
    );
    // Constructor to load from file (will need to parse layer info or use defaults)
    Tilemap(
        Spritesheet* sheet, int tile_width, int tile_height, int map_width,
        int map_height, const std::map<int, CollisionLayer>& tile_collision_layers,
        const char* path
    );
    ~Tilemap();

    void setTile(int tileX, int tileY, int tile_index);
    int getTile(int tileX, int tileY) const;
    CollisionLayer getTileLayer(int tileX, int tileY) const; // Get layer of a tile

    void draw(
        SDL_Renderer* renderer, int dest_x, int dest_y, int dest_w = -1,
        int dest_h = -1
    ) const;

    // New collision check function using layers and masks
    // Takes a proposed bounding box and the entity's collision mask
    // Returns true if a collision occurs with a relevant tile layer.
    bool checkCollision(const SDL_FRect& boundingBox, CollisionLayer entityMask) const;

    // Old intersects_rect - Deprecated or adapt if needed
    // Direction intersects_rect(float x, float y, float w, float h) const;

    // Raycast might need updating to consider layers too
    float raycast(float x, float y, float angle) const;

private:
    Spritesheet* sheet;
    int tile_width;
    int tile_height;
    int map_width;
    int map_height;
    std::vector<int> tiles; // Use std::vector for easier management
    std::map<int, CollisionLayer> tileCollisionLayers; // Map tile index -> layer

    // Helper to load map data from a text file
    void loadFromFile(const char* path);
    // Helper to save map data (optional)
    // void saveToFile(const char* path) const;

    // Internal helper to check if a specific tile index has a collision layer
    bool isTileCollidable(int tileIndex, CollisionLayer entityMask) const;
};
