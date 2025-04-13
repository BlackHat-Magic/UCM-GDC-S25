// src/utils/tilemap.cpp
#include "tilemap.h"
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <SDL2/SDL.h>
#include "spritesheet.h"
#include <iostream>
#include <cmath> // For std::floor, std::ceil, std::abs, std::max, std::min

// Constructor implementation
Tilemap::Tilemap(
    Spritesheet* sheet, int tile_width, int tile_height, int map_width,
    int map_height, const std::map<int, CollisionLayer>& tile_collision_layers
) :
    sheet(sheet),
    tile_width(tile_width),
    tile_height(tile_height),
    map_width(map_width),
    map_height(map_height),
    tiles(map_width * map_height, 0), // Initialize vector with 0 (empty tile)
    tileCollisionLayers(tile_collision_layers), // Copy the layer map
    opacity(1.0f) // Default to fully opaque
{
    if (!sheet) {
        // Allow null spritesheet for non-renderable collision maps? Or throw?
        // Let's throw for now, as drawing is expected.
        throw std::runtime_error("Tilemap created with null spritesheet.");
    }
    if (tile_width <= 0 || tile_height <= 0 || map_width <= 0 ||
        map_height <= 0) {
        throw std::runtime_error("Invalid Tilemap dimensions.");
    }
}

// REMOVED: Constructor that loads from simple file
// Tilemap::Tilemap(...)

Tilemap::~Tilemap() {
    // No manual memory management needed for std::vector or std::map
    // Spritesheet ownership is assumed to be external (managed by shared_ptr in parser)
}

// Set tile data at given tile coordinates (stores local ID | flags)
void Tilemap::setTile(int tileX, int tileY, int tile_value) {
    if (tileX >= 0 && tileX < map_width && tileY >= 0 && tileY < map_height) {
        tiles[tileY * map_width + tileX] = tile_value;
    } else {
         // Optional: Log warning for out-of-bounds setTile
         // std::cerr << "Warning: setTile out of bounds (" << tileX << ", " << tileY << ")" << std::endl;
    }
}

// Get tile value (local ID | flags) at given tile coordinates
int Tilemap::getTile(int tileX, int tileY) const {
    if (tileX >= 0 && tileX < map_width && tileY >= 0 && tileY < map_height) {
        return tiles[tileY * map_width + tileX];
    }
    return 0; // Return 0 (empty) for out of bounds
}

// Get collision layer of a tile at given coordinates
CollisionLayer Tilemap::getTileLayer(int tileX, int tileY) const {
     int tileValue = getTile(tileX, tileY);
     if (tileValue != 0) { // Check non-empty tiles
         int localTileId = extractTileId(tileValue); // Use local ID for lookup
         auto it = tileCollisionLayers.find(localTileId);
         if (it != tileCollisionLayers.end()) {
             return it->second; // Return the layer from the map
         }
     }
     return CollisionLayer::NONE; // No specific layer defined for this tile or empty
}

// Render the tilemap
void Tilemap::draw(
    SDL_Renderer* renderer, int dest_x, int dest_y, int dest_w, int dest_h
) const {
    if (!sheet) {
         std::cerr << "Warning: Attempting to draw Tilemap with null spritesheet." << std::endl;
         return; // Don't draw if spritesheet is missing
    }

    int drawTileW = (dest_w == -1) ? tile_width : dest_w;
    int drawTileH = (dest_h == -1) ? tile_height : dest_h;

    SDL_Texture* texture = sheet->getTexture();
    if (!texture) {
         std::cerr << "Warning: Spritesheet texture is null in Tilemap::draw." << std::endl;
         return; // Don't draw if texture is missing
    }

    // Store original alpha/blend to restore later
    Uint8 originalAlpha = 255;
    SDL_BlendMode originalBlendMode;
    bool stateChanged = false;

    if (opacity < 1.0f) {
        SDL_GetTextureAlphaMod(texture, &originalAlpha);
        SDL_GetTextureBlendMode(texture, &originalBlendMode);
        SDL_SetTextureAlphaMod(texture, static_cast<Uint8>(opacity * 255));
        SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);
        stateChanged = true;
    } else {
        // Ensure texture is fully opaque if opacity is 1.0 or more
        // This might be redundant if default is 255, but safe.
        // SDL_SetTextureAlphaMod(texture, 255);
    }


    for (int y = 0; y < map_height; ++y) {
        for (int x = 0; x < map_width; ++x) {
            int tileValue = getTile(x, y);
            if (tileValue != 0) { // Only draw non-empty tiles
                // Extract the actual tile ID and flip flags using static helpers
                int localTileId = Tilemap::extractTileId(tileValue);
                SDL_RendererFlip flip = Tilemap::extractFlipFlags(tileValue);

                // Tiled uses 0 for empty, but tile IDs are 0-based from firstGid.
                // Spritesheet indices are also 0-based.
                // So a localTileId of 0 is valid if it's the first tile in the tileset.

                int drawPosX = dest_x + x * drawTileW;
                int drawPosY = dest_y + y * drawTileH;

                try {
                    sheet->select_sprite(localTileId); // Use local ID
                    sheet->draw(renderer, drawPosX, drawPosY, drawTileW, drawTileH, flip); // Pass flip flags
                } catch (const std::out_of_range& oor) {
                     std::cerr << "Tilemap draw error: Tile index " << localTileId
                               << " (from raw value " << tileValue << ")"
                               << " out of range for spritesheet." << std::endl;
                     // Optionally draw an error tile or skip
                } catch (const std::exception& e) {
                     std::cerr << "Tilemap draw error: " << e.what() << std::endl;
                }
            }
        }
    }

    // Reset alpha and blend mode if we changed them
    if (stateChanged) {
        SDL_SetTextureAlphaMod(texture, originalAlpha);
        SDL_SetTextureBlendMode(texture, originalBlendMode);
    }
}

// REMOVED: loadFromFile function

// Internal helper to check if a specific LOCAL tile index collides with a given mask
bool Tilemap::isTileCollidable(int localTileId, CollisionLayer entityMask) const {
    if (localTileId < 0) return false; // Should not happen with unsigned GIDs, but safety check

    auto it = tileCollisionLayers.find(localTileId);
    if (it != tileCollisionLayers.end()) {
        // Check if the tile's layer interacts with the entity's mask
        return ::checkCollision(entityMask, it->second);
    }
    return false; // Tile index not found in collision map, assume non-collidable
}

// New collision check function
bool Tilemap::checkCollision(const SDL_FRect& boundingBox, CollisionLayer entityMask) const {
    // Convert world coordinates (boundingBox) to tile coordinates
    // Note: Assumes boundingBox x,y is top-left. Adjust if it's center.
    int startTileX = static_cast<int>(std::floor(boundingBox.x / tile_width));
    int endTileX = static_cast<int>(std::floor((boundingBox.x + boundingBox.w - 0.001f) / tile_width)); // Subtract epsilon for edge cases
    int startTileY = static_cast<int>(std::floor(boundingBox.y / tile_height));
    int endTileY = static_cast<int>(std::floor((boundingBox.y + boundingBox.h - 0.001f) / tile_height)); // Subtract epsilon for edge cases

    // Clamp tile coordinates to map bounds
    startTileX = std::max(0, startTileX);
    endTileX = std::min(map_width - 1, endTileX);
    startTileY = std::max(0, startTileY);
    endTileY = std::min(map_height - 1, endTileY);

    // Check all tiles the bounding box overlaps
    for (int ty = startTileY; ty <= endTileY; ++ty) {
        for (int tx = startTileX; tx <= endTileX; ++tx) {
            int tileValue = getTile(tx, ty);
            if (tileValue != 0) { // Check valid, non-empty tiles
                // Extract the actual local tile ID (ignore flip flags for collision)
                int localTileId = Tilemap::extractTileId(tileValue);
                if (isTileCollidable(localTileId, entityMask)) {
                    // Found a collision with a relevant tile layer
                    return true;
                }
            }
        }
    }

    return false; // No collision found
}

// Raycast function - might need updating to consider layers
float Tilemap::raycast(float x, float y, float angle) const {
    // Convert start position to tile space (assuming x,y is center)
    float startTileX = x / tile_width;
    float startTileY = y / tile_height;

    float rayDirX = std::cos(angle);
    float rayDirY = std::sin(angle);

    // Current tile coordinates
    int mapX = static_cast<int>(std::floor(startTileX)); // Use floor for initial map coords
    int mapY = static_cast<int>(std::floor(startTileY));

    // Length of ray from current position to next x or y-side
    float sideDistX, sideDistY;

    // Length of ray from one x or y-side to next x or y-side
    // Avoid division by zero
    float deltaDistX = (rayDirX == 0) ? 1e30f : std::abs(1.0f / rayDirX);
    float deltaDistY = (rayDirY == 0) ? 1e30f : std::abs(1.0f / rayDirY);
    float perpWallDist;

    // Which direction to step in x or y-direction (either +1 or -1)
    int stepX, stepY;

    bool hit = false; // Was there a wall hit?
    int side;         // Was a NS or a EW wall hit?

    // Calculate step and initial sideDist
    if (rayDirX < 0) {
        stepX = -1;
        sideDistX = (startTileX - mapX) * deltaDistX;
    } else {
        stepX = 1;
        sideDistX = (mapX + 1.0f - startTileX) * deltaDistX;
    }
    if (rayDirY < 0) {
        stepY = -1;
        sideDistY = (startTileY - mapY) * deltaDistY;
    } else {
        stepY = 1;
        sideDistY = (mapY + 1.0f - startTileY) * deltaDistY;
    }

    // Perform DDA (Digital Differential Analysis)
    while (!hit) {
        // Jump to next map square, OR in x-direction, OR in y-direction
        if (sideDistX < sideDistY) {
            sideDistX += deltaDistX;
            mapX += stepX;
            side = 0; // Hit vertical wall line
        } else {
            sideDistY += deltaDistY;
            mapY += stepY;
            side = 1; // Hit horizontal wall line
        }

        // Check if ray is outside map bounds
        if (mapX < 0 || mapX >= map_width || mapY < 0 || mapY >= map_height) {
            return -1.0f; // Hit map boundary (or went to infinity)
        }

        // Check if ray has hit a collidable tile
        int tileValue = getTile(mapX, mapY);
        if (tileValue != 0) { // Check non-empty tiles
            int localTileId = Tilemap::extractTileId(tileValue);
            // *** TODO: Update this check to use isTileCollidable with an appropriate mask ***
            // For now, just checks if the tile exists in the collision layer map
            // A proper implementation would pass the entity's mask or a specific raycast mask
            // e.g. if (isTileCollidable(localTileId, CollisionLayer::MASK_RAYCAST_TARGETS)) { hit = true; }
             if (tileCollisionLayers.count(localTileId)) { // Simple check if ID is in the map
                 // More precise: Check if the tile *at this location* is collidable
                 // CollisionLayer tileLayer = getTileLayer(mapX, mapY);
                 // if (::checkCollision(SOME_RAYCAST_MASK, tileLayer)) { hit = true; }
                 hit = true; // Assuming any tile in the map is collidable for raycast for now
             }
        }
    }

    // Calculate distance projected on camera direction (Euclidean distance would be sqrt(distSq))
    if (side == 0) {
        perpWallDist = (sideDistX - deltaDistX);
    } else {
        perpWallDist = (sideDistY - deltaDistY);
    }

    // Convert distance back to world space
    // Assuming square tiles for simplicity, otherwise use average or specific axis
    return perpWallDist * tile_width;
}
