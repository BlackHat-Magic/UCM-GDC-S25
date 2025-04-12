#pragma once
#include<SDL2/SDL_image.h>
#include<SDL2/SDL.h>

class Spritesheet {
public:
	// width and height are width/height of sprites
	Spritesheet(SDL_Renderer *renderer, char const *path, int width, int height);
	~Spritesheet();

	void select_sprite(int i);
	// draw sprite sheet on provided renderer
	void draw(SDL_Renderer *renderer, int dest_x, int dest_y, int dest_w = -1, int dest_h = -1);

private:
	SDL_Texture *texture;
	SDL_Rect src_rect;

	int sprite_width, sprite_height;
	int sheet_width, sheet_height;
	int rows, cols;
};
