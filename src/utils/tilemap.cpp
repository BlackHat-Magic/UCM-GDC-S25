#include "tilemap.h"
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <SDL2/SDL.h>
#include "spritesheet.h"
#include <iostream>
#include <cmath> // For std::floor, std::ceil, std::abs

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
    tiles(map_width * map_height, -1), // Initialize vector with -1
    tileCollisionLayers(tile_collision_layers) // Copy the layer map
{
    if (!sheet) {
        throw std::runtime_error("Tilemap created with null spritesheet.");
    }
    if (tile_width <= 0 || tile_height <= 0 || map_width <= 0 ||
        map_height <= 0) {
        throw std::runtime_error("Invalid Tilemap dimensions.");
    }
}

// Constructor that loads from file
Tilemap::Tilemap(
    Spritesheet* sheet, int tile_width, int tile_height, int map_width,
    int map_height, const std::map<int, CollisionLayer>& tile_collision_layers,
    const char* path
) :
    Tilemap(
        sheet, tile_width, tile_height, map_width, map_height,
        tile_collision_layers
    ) // Delegate to the other constructor
{
    loadFromFile(path); // Load data after basic initialization
}

Tilemap::~Tilemap() {
    // No manual memory management needed for std::vector or std::map
    // Spritesheet ownership is assumed to be external
}

// Set tile data at given tile coordinates
void Tilemap::setTile(int tileX, int tileY, int tile_index) {
    if (tileX >= 0 && tileX < map_width && tileY >= 0 && tileY < map_height) {
        tiles[tileY * map_width + tileX] = tile_index;
    }
}

// Get tile index at given tile coordinates
int Tilemap::getTile(int tileX, int tileY) const {
    if (tileX >= 0 && tileX < map_width && tileY >= 0 && tileY < map_height) {
        return tiles[tileY * map_width + tileX];
    }
    return -1; // Out of bounds or empty
}

// Get collision layer of a tile at given coordinates
CollisionLayer Tilemap::getTileLayer(int tileX, int tileY) const {
     int tileIndex = getTile(tileX, tileY);
     if (tileIndex != -1) {
         auto it = tileCollisionLayers.find(tileIndex);
         if (it != tileCollisionLayers.end()) {
             return it->second; // Return the layer from the map
         }
     }
     return CollisionLayer::NONE; // No specific layer defined for this tile
}


// Render the tilemap
void Tilemap::draw(
    SDL_Renderer* renderer, int dest_x, int dest_y, int dest_w, int dest_h
) const {
    int drawTileW = (dest_w == -1) ? tile_width : dest_w;
    int drawTileH = (dest_h == -1) ? tile_height : dest_h;

    for (int y = 0; y < map_height; ++y) {
        for (int x = 0; x < map_width; ++x) {
            int tile_index = getTile(x, y);
            if (tile_index != -1) { // Only draw valid tiles
                int drawPosX = dest_x + x * drawTileW;
                int drawPosY = dest_y + y * drawTileH;

                try {
                    sheet->select_sprite(tile_index);
                    sheet->draw(renderer, drawPosX, drawPosY, drawTileW, drawTileH);
                } catch (const std::out_of_range& oor) {
                     std::cerr << "Tilemap draw error: Tile index " << tile_index
                               << " out of range for spritesheet." << std::endl;
                     // Optionally draw an error tile or skip
                }
            }
        }
    }
}

// Load map data from a text file
void Tilemap::loadFromFile(const char* path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        throw std::runtime_error(
            std::string("Failed to open tilemap file: ") + path
        );
    }

    std::string line;
    int y = 0;
    while (std::getline(file, line) && y < map_height) {
        std::stringstream ss(line);
        int x = 0;
        int tile_index;
        while (ss >> tile_index && x < map_width) {
            setTile(x, y, tile_index);
            x++;
        }
        y++;
    }

    if (y < map_height) {
         std::cerr << "Warning: Tilemap file " << path << " has fewer rows (" << y
                   << ") than expected (" << map_height << ")." << std::endl;
    }

    file.close();
}

// Internal helper to check if a specific tile index collides with a given mask
bool Tilemap::isTileCollidable(int tileIndex, CollisionLayer entityMask) const {
    if (tileIndex == -1) return false; // Empty tile

    auto it = tileCollisionLayers.find(tileIndex);
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
    int endTileX = static_cast<int>(std::floor((boundingBox.x + boundingBox.w) / tile_width));
    int startTileY = static_cast<int>(std::floor(boundingBox.y / tile_height));
    int endTileY = static_cast<int>(std::floor((boundingBox.y + boundingBox.h) / tile_height));

    // Clamp tile coordinates to map bounds
    startTileX = std::max(0, startTileX);
    endTileX = std::min(map_width - 1, endTileX);
    startTileY = std::max(0, startTileY);
    endTileY = std::min(map_height - 1, endTileY);

    // Check all tiles the bounding box overlaps
    for (int ty = startTileY; ty <= endTileY; ++ty) {
        for (int tx = startTileX; tx <= endTileX; ++tx) {
            int tileIndex = getTile(tx, ty);
            if (isTileCollidable(tileIndex, entityMask)) {
                // Found a collision with a relevant tile layer
                return true;
            }
        }
    }

    return false; // No collision found
}


// --- Deprecated / Needs Update ---
/*
Direction Tilemap::intersects_rect(float x, float y, float w, float h) const {
    // This function needs significant rework to use layers/masks and provide
    // useful collision response information (like normals).
    // Returning Direction is not ideal for velocity-based movement.
    // Keeping the old implementation commented out for reference.

    // ... (old implementation) ...

    return NONE; // Placeholder
}
*/

// Raycast function - might need updating to consider layers
float Tilemap::raycast(float x, float y, float angle) const {
    // Convert start position to tile space (assuming x,y is center)
    float startTileX = x / tile_width;
    float startTileY = y / tile_height;

    float rayDirX = std::cos(angle);
    float rayDirY = std::sin(angle);

    // Current tile coordinates
    int mapX = static_cast<int>(startTileX);
    int mapY = static_cast<int>(startTileY);

    // Length of ray from current position to next x or y-side
    float sideDistX, sideDistY;

    // Length of ray from one x or y-side to next x or y-side
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
        // *** TODO: Update this check to use isTileCollidable with an appropriate mask ***
        // For now, just checks if the tile exists in the collision layer map
        int hitTileIndex = getTile(mapX, mapY);
        if (hitTileIndex != -1 && tileCollisionLayers.count(hitTileIndex)) {
             // Assuming any tile in the map is collidable for raycast for now
             // A proper implementation would pass the entity's mask
             // if (isTileCollidable(hitTileIndex, MASK_RAYCAST_TARGETS)) { hit = true; }
             hit = true;
        }
    }

    // Calculate distance projected on camera direction (Euclidean distance would be sqrt(distSq))
    if (side == 0) {
        perpWallDist = (sideDistX - deltaDistX);
    } else {
        perpWallDist = (sideDistY - deltaDistY);
    }

    // Convert distance back to world space
    return perpWallDist * tile_width; // Assumes square tiles for simplicity
}
