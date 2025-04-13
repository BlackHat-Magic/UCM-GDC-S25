#pragma once
#include <SDL2/SDL.h>
#include "spritesheet.h"
#include "direction.h"


class Tilemap {
public:
    Tilemap(Spritesheet *sheet, int tile_width, int tile_height, int map_width, int map_height, int* tiles_with_collider);
    Tilemap(Spritesheet *sheet, int tile_width, int tile_height, int map_width, int map_height, int* tiles_with_collider, const char *path);
    ~Tilemap();

    void setTile(int x, int y, int tile_index);
    int getTile(int x, int y) const;

    void draw(SDL_Renderer *renderer, float cameraX, float cameraY, int dest_w = -1, int dest_h = -1) const;
    Direction intersects_rect(float x, float y, float w, float h) const;
    // angle is in radians
    float raycast(float x, float y, float angle) const;

private:
    Spritesheet *sheet;
    int tile_width;
    int tile_height;
    int map_width;
    int map_height;
    int *tiles;
    int *tiles_with_collider;
    long long* collider;

    void loadFromFile(const char *path);
    void saveToFile(const char *path) const;
};