// src/utils/tilemap.h
#pragma once
#include <SDL2/SDL.h>
#include <vector>
#include <map> // For mapping tile index to layer
#include "spritesheet.h"
#include "collisions_defs.h" // Include collision definitions
#include "direction.h"       // Keep for now if needed elsewhere
#include <cmath> // Required for std::floor, std::max, std::min
#include <iostream> // Required for std::cerr
#include <stdexcept> // For runtime_error

class Tilemap {
public:
    // --- Tiled Flip Flags ---
    // Stored in the most significant bits of the GID
    static const unsigned FLIPPED_HORIZONTALLY_FLAG = 0x80000000;
    static const unsigned FLIPPED_VERTICALLY_FLAG   = 0x40000000;
    static const unsigned FLIPPED_DIAGONALLY_FLAG   = 0x20000000;
    // Mask to clear all flip flags and get the actual GID
    static const unsigned GID_MASK = ~(FLIPPED_HORIZONTALLY_FLAG | FLIPPED_VERTICALLY_FLAG | FLIPPED_DIAGONALLY_FLAG);

    // Constructor now takes a map defining which tile indices map to which collision layers
    Tilemap(
        Spritesheet* sheet, int tile_width, int tile_height, int map_width,
        int map_height, const std::map<int, CollisionLayer>& tile_collision_layers
    );
    // REMOVED: Constructor to load from simple file (not suitable for TMX)
    // Tilemap(
    //     Spritesheet* sheet, int tile_width, int tile_height, int map_width,
    //     int map_height, const std::map<int, CollisionLayer>& tile_collision_layers,
    //     const char* path
    // );
    ~Tilemap();

    void setTile(int tileX, int tileY, int tile_value); // tile_value includes flags
    int getTile(int tileX, int tileY) const; // Returns value including flags
    CollisionLayer getTileLayer(int tileX, int tileY) const; // Get layer of a tile

    void draw(
        SDL_Renderer* renderer, int dest_x, int dest_y, int dest_w = -1,
        int dest_h = -1
    ) const;

    // New collision check function using layers and masks
    bool checkCollision(const SDL_FRect& boundingBox, CollisionLayer entityMask) const;

    // Set opacity for rendering (0.0f to 1.0f)
    void setOpacity(float opacity) { this->opacity = opacity; }
    float getOpacity() const { return opacity; }

    // Raycast might need updating to consider layers too
    float raycast(float x, float y, float angle) const;

    // --- Static Helper methods for handling flipped tiles ---

    // Extract the actual tile ID (local to the spritesheet) from a stored tile value
    static int extractTileId(int tileValue) {
        return tileValue & GID_MASK; // Apply mask to remove flags
    }

    // Extract the flip flags from a stored tile value
    static SDL_RendererFlip extractFlipFlags(int tileValue) {
        SDL_RendererFlip flip = SDL_FLIP_NONE;

        if (tileValue & FLIPPED_HORIZONTALLY_FLAG) {
            flip = static_cast<SDL_RendererFlip>(flip | SDL_FLIP_HORIZONTAL);
        }
        if (tileValue & FLIPPED_VERTICALLY_FLAG) {
            flip = static_cast<SDL_RendererFlip>(flip | SDL_FLIP_VERTICAL);
        }
        // Note: Diagonal flipping is more complex and often requires rotation + flipping.
        // SDL_RenderCopyEx doesn't directly support diagonal flipping.
        // We ignore the diagonal flag here for simplicity with SDL_RenderCopyEx.
        // if (tileValue & FLIPPED_DIAGONALLY_FLAG) {
        //     // Handle diagonal flip (e.g., requires rotation and potentially other flips)
        // }

        return flip;
    }


private:
    Spritesheet* sheet;
    int tile_width;
    int tile_height;
    int map_width;
    int map_height;
    std::vector<int> tiles; // Stores LOCAL tile ID | flip flags
    std::map<int, CollisionLayer> tileCollisionLayers; // Map LOCAL tile index -> layer
    float opacity; // Opacity for rendering (0.0f to 1.0f)

    // REMOVED: Helper to load map data from a simple text file
    // void loadFromFile(const char* path);

    // Internal helper to check if a specific LOCAL tile index has a collision layer
    bool isTileCollidable(int localTileId, CollisionLayer entityMask) const;

};
