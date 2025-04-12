#pragma once
#include <SDL2/SDL.h>
#include "spritesheet.h"

class Tilemap {
public:
    Tilemap(Spritesheet *sheet, int tile_width, int tile_height, int map_width, int map_height);
    Tilemap(Spritesheet *sheet, int tile_width, int tile_height, int map_width, int map_height, const char *path);
    ~Tilemap();

    void setTile(int x, int y, int tile_index);
    int getTile(int x, int y) const;

    void draw(SDL_Renderer *renderer, int dest_x, int dest_y, int dest_w = -1, int dest_h = -1) const;

private:
    Spritesheet *sheet;
    int tile_width;
    int tile_height;
    int map_width;
    int map_height;
    int *tiles;

    void loadFromFile(const char *path);
    void saveToFile(const char *path) const;
};