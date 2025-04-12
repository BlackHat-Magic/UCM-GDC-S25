#include "spritesheet.h"
#include<stdexcept>
#include <iostream>

Spritesheet::Spritesheet(SDL_Renderer *renderer, const char *path, int width, int height) {
	texture = IMG_LoadTexture(renderer, path);
	if (!texture) {
		std::cerr << "Failed to load texture: " << path << std::endl;
		std::cerr << "SDL_image Error: " << IMG_GetError() << std::endl;

		throw std::runtime_error("Failed to load texture");
	}

	SDL_QueryTexture(texture, NULL, NULL, &sheet_width, &sheet_height);

	sprite_width = width;
	sprite_height = height;

	// how wide/tall is it in sprite terms
	cols = sheet_width / sprite_width;
	rows = sheet_height / sprite_height;

	src_rect.x = 0;
	src_rect.y = 0;
	src_rect.w = sprite_width;
	src_rect.h = sprite_height;
}

Spritesheet::~Spritesheet() {
	SDL_DestroyTexture(texture);
}

void Spritesheet::select_sprite(int i) {
	if (i < 0 || i >= rows * cols)
		throw std::out_of_range("Sprite index out of range");

	int x = i % cols;
	int y = i / cols;

	src_rect.x = x * sprite_width;
	src_rect.y = y * sprite_height;
}

// draw sprite on provided renderer
void Spritesheet::draw(SDL_Renderer *renderer, int dest_x, int dest_y, int dest_w, int dest_h, SDL_RendererFlip flip) {
	SDL_Rect dest_rect;
	// position to render at
	dest_rect.x = dest_x;
	dest_rect.y = dest_y;
	// dimensions of sprite to draw
	dest_rect.w = (dest_w == -1) ? sprite_width : dest_w;
	dest_rect.h = (dest_h == -1) ? sprite_height : dest_h;

	SDL_RenderCopyEx(renderer, texture, &src_rect, &dest_rect, 0, NULL, flip);
}
