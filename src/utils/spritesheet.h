#pragma once
#include<SDL2/SDL_image.h>
#include<SDL2/SDL.h>

class Spritesheet {
public:
	Spritesheet(SDL_Renderer *renderer, char const *path, int width, int height);
	~Spritesheet();

	void select_sprite(int i);
	void draw(SDL_Renderer *renderer, int dest_x, int dest_y, int dest_w = -1, int dest_h = -1);

private:
	SDL_Texture *texture;
	SDL_Rect src_rect;

	int sprite_width, sprite_height;
	int sheet_width, sheet_height;
	int rows, cols;
};
