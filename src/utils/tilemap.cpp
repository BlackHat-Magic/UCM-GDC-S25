#include "tilemap.h"
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <SDL2/SDL.h>
#include "spritesheet.h"

Tilemap::Tilemap(Spritesheet *sheet, int tile_width, int tile_height, int map_width, int map_height)
    : sheet(sheet), tile_width(tile_width), tile_height(tile_height), map_width(map_width), map_height(map_height) {
    tiles = new int[map_width * map_height];
    std::fill(tiles, tiles + (map_width * map_height), -1);
}

Tilemap::Tilemap(Spritesheet *sheet, int tile_width, int tile_height, int map_width, int map_height, const char *path)
    : Tilemap(sheet, tile_width, tile_height, map_width, map_height) {
    loadFromFile(path);
}

Tilemap::~Tilemap() {
    delete[] tiles;
}

void Tilemap::setTile(int x, int y, int tile_index) {
    if (x >= 0 && x < map_width && y >= 0 && y < map_height) {
        tiles[y * map_width + x] = tile_index;
    }
}

int Tilemap::getTile(int x, int y) const {
    if (x >= 0 && x < map_width && y >= 0 && y < map_height) {
        return tiles[y * map_width + x];
    }
    return -1;
}

void Tilemap::draw(SDL_Renderer *renderer, int dest_x, int dest_y, int dest_w, int dest_h) const {
    for (int y = 0; y < map_height; ++y) {
        for (int x = 0; x < map_width; ++x) {
            int tile_index = getTile(x, y);
            if (tile_index != -1) {
                int pos_x = dest_x + x * tile_width;
                int pos_y = dest_y + y * tile_height;

                int tile_width = (dest_w == -1) ? tile_width : dest_w;
                int tile_height = (dest_h == -1) ? tile_height : dest_h;

                sheet->select_sprite(tile_index);
                sheet->draw(renderer, pos_x, pos_y, tile_width, tile_height);
            }
        }
    }
}

void Tilemap::loadFromFile(const char *path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open tilemap file.");
    }

    std::string line;
    int y = 0;

    while (std::getline(file, line) && y < map_height) {
        std::stringstream ss(line);
        int x = 0;
        int tile_index;

        while (ss >> tile_index && x < map_width) {
            setTile(x, y, tile_index);
            ++x;
        }
        ++y;
    }

    file.close();
}

void Tilemap::saveToFile(const char *path) const {
    std::ofstream file(path);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open tilemap file for saving.");
    }

    for (int y = 0; y < map_height; ++y) {
        for (int x = 0; x < map_width; ++x) {
            file << getTile(x, y);
            if (x < map_width - 1) {
                file << " ";
            }
        }
        file << std::endl;
    }

    file.close();
}
