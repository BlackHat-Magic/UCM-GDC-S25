#include "tilemap.h"
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <SDL2/SDL.h>
#include "spritesheet.h"
#include <iostream>

// creates a tilemap object with specified dimensions
Tilemap::Tilemap(Spritesheet *sheet, int tile_width, int tile_height, int map_width, int map_height, int *tiles_with_collider)
    : sheet(sheet), tile_width(tile_width), tile_height(tile_height), map_width(map_width), map_height(map_height), 
      tiles_with_collider(tiles_with_collider) {
    tiles = new int[map_width * map_height];
    std::fill(tiles, tiles + (map_width * map_height), -1);

    collider = new long long[(map_width * map_height + 63) / 64];
}

// same as above but also loads map data from file
Tilemap::Tilemap(Spritesheet *sheet, int tile_width, int tile_height, int map_width, int map_height, int *tiles_with_collider, const char *path)
    : Tilemap(sheet, tile_width, tile_height, map_width, map_height, tiles_with_collider) {
    loadFromFile(path);
}

Tilemap::~Tilemap() {
    delete[] tiles;
    // delete[] tiles_with_collider;
    delete[] collider;
}

// change the data at the given coordinates
void Tilemap::setTile(int x, int y, int tile_index) {
    if (x >= 0 && x < map_width && y >= 0 && y < map_height) {
        tiles[y * map_width + x] = tile_index;
        
        for (int i = 0; tiles_with_collider[i] != -1; ++i) {
            if (tile_index == tiles_with_collider[i]) {
                int index = y * map_width + x;
                collider[index / 64] |= (1LL << (index % 64));
            } else {
                int index = y * map_width + x;
                collider[index / 64] &= ~(1LL << (index % 64));
            }
        }
    }
}

int Tilemap::getTile(int x, int y) const {
    if (x >= 0 && x < map_width && y >= 0 && y < map_height) {
        return tiles[y * map_width + x];
    }
    return -1;
}

// render the entire visible portion of the tilemap
void Tilemap::draw(SDL_Renderer *renderer, int dest_x, int dest_y, int dest_w, int dest_h) const {
    for (int y = 0; y < map_height; ++y) {
        for (int x = 0; x < map_width; ++x) {
            int tile_index = getTile(x, y);
            if (tile_index != -1) {
                int width = (dest_w == -1) ? tile_width : dest_w;
                int height = (dest_h == -1) ? tile_height : dest_h;

                int pos_x = dest_x + x * width;
                int pos_y = dest_y + y * height;

                sheet->select_sprite(tile_index);
                sheet->draw(renderer, pos_x, pos_y, width, height);
            }
        }
    }
}

// load map data from text file
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

Direction Tilemap::intersects_rect(float x, float y, float w, float h) const {
    // convert from world space to tile space
    x = x / tile_width;
    y = y / tile_height;
    w = w / tile_width;
    h = h / tile_height;

    Direction dir = NONE;
    int left = static_cast<int>(x - w / 2.0f);
    int right = static_cast<int>(x + w / 2.0f);
    int top = static_cast<int>(y - h / 2.0f);
    int bottom = static_cast<int>(y + h / 2.0f);

    for (int i = left; i <= right; ++i) {
        for (int j = top; j <= bottom; ++j) {
            if (i < 0 || j < 0 || i >= map_width || j >= map_height) {
                continue;
            }

            int index = j * map_width + i;
            long long byte = collider[index / 64];
            long long bit = (byte >> (index % 64)) & 1;

            if (bit == 1) {
                float i_f = static_cast<float>(i) + 0.5f;
                float j_f = static_cast<float>(j) + 0.5f;
                float x_dif = std::abs(x - i_f);
                float y_dif = std::abs(y - j_f);

                if (x_dif > y_dif) {
                    if (dir == UP) {
                        dir = (x > i_f) ? UP_LEFT : UP_RIGHT;
                        return dir;
                    }
                    if (dir == DOWN) {
                        dir = (x > i_f) ? DOWN_LEFT : DOWN_RIGHT;
                        return dir;
                    }
                    dir = (x > i_f) ? LEFT : RIGHT;
                } else {
                    if (dir == LEFT) {
                        dir = (y > j_f) ? DOWN_LEFT : UP_LEFT;
                        return dir;
                    }
                    if (dir == RIGHT) {
                        dir = (y > j_f) ? DOWN_RIGHT : UP_RIGHT;
                        return dir;
                    }
                    dir = (y > j_f) ? UP : DOWN;
                }
            }
        }
    }

    return dir;
}

float Tilemap::raycast(float x, float y, float angle) const {
    // convert from world space to tile space
    x = x / tile_width;
    y = y / tile_height;

    float raydir_x = std::cos(angle);
    float raydir_y = std::sin(angle);

    int map_x = static_cast<int>(x);
    int map_y = static_cast<int>(y);

    float delta_dist_x = (raydir_x == 0) ? 1e30f : std::abs(1.0f / raydir_x);
    float delta_dist_y = (raydir_y == 0) ? 1e30f : std::abs(1.0f / raydir_y);
    float side_dist_x, side_dist_y;
    int step_x, step_y;
    if (raydir_x < 0) {
        step_x = -1;
        side_dist_x = (x - static_cast<float>(map_x)) * delta_dist_x;
    } else {
        step_x = 1;
        side_dist_x = (static_cast<float>(map_x + 1) - x) * delta_dist_x;
    }
    if (raydir_y < 0) {
        step_y = -1;
        side_dist_y = (y - static_cast<float>(map_y)) * delta_dist_y;
    } else {
        step_y = 1;
        side_dist_y = (static_cast<float>(map_y + 1) - y) * delta_dist_y;
    }
    bool hit = false;
    int side = 0; // 0: X-side, 1: Y-side
    while (!hit) {
        if (side_dist_x < side_dist_y) {
            side_dist_x += delta_dist_x;
            map_x += step_x;
            side = 0;
        } else {
            side_dist_y += delta_dist_y;
            map_y += step_y;
            side = 1;
        }

        if (map_x < 0 || map_x >= map_width || map_y < 0 || map_y >= map_height) {
            return -1.0f;
        }

        int index = map_y * map_width + map_x;
        long long byte = collider[index / 64];
        long long bit = (byte >> (index % 64)) & 1;

        if (bit == 1) {
            hit = true;
        }
    }

    float perp_wall_dist;
    if (side == 0) {
        perp_wall_dist = (static_cast<float>(map_x) - x + (1.0f - static_cast<float>(step_x)) / 2.0f) / raydir_x;
    } else {
        perp_wall_dist = (static_cast<float>(map_y) - y + (1.0f - static_cast<float>(step_y)) / 2.0f) / raydir_y;
    }
    // convert dist to world space
    perp_wall_dist *= tile_width; // technically doesn't work if tile_width != tile_height, but uh, shut up
    return perp_wall_dist;
}