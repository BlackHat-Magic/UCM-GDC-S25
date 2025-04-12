#include "spritesheet.h"
#include<stdexcept>

Spritesheet::Spritesheet(SDL_Renderer *renderer, const char *path, int width, int height) {
	texture = IMG_LoadTexture(renderer, path);
	if (!texture)
		throw std::runtime_error("Failed to load texture");

	SDL_QueryTexture(texture, NULL, NULL, &sheet_width, &sheet_height);

	sprite_width = width;
	sprite_height = height;

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

void Spritesheet::select_sprite(int x, int y) {
	if (x < 0 || x >= cols || y < 0 || y >= rows)
		throw std::out_of_range("Sprite index out of range");

	src_rect.x = x * sprite_width;
	src_rect.y = y * sprite_height;
}

void Spritesheet::draw(SDL_Renderer *renderer, int dest_x, int dest_y, int dest_w, int dest_h) {
	SDL_Rect dest_rect;
	dest_rect.x = dest_x;
	dest_rect.y = dest_y;
	dest_rect.w = (dest_w == -1) ? sprite_width : dest_w;
	dest_rect.h = (dest_h == -1) ? sprite_height : dest_h;

	SDL_RenderCopy(renderer, texture, &src_rect, &dest_rect);
}
