#include <SDL2/SDL.h>
#include "utils/spritesheet.h"
#include "utils/audio.h"
#include "utils/tilemap.h"
#include "utils/input.h"
#include <iostream>

int main() {
	bool quit = false;
	SDL_Event event;

	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		return 1;
	}

	SDL_Window* window = SDL_CreateWindow("Hello SDL2",
										  SDL_WINDOWPOS_UNDEFINED,
										  SDL_WINDOWPOS_UNDEFINED,
										  640, 480,
										  SDL_WINDOW_SHOWN);
	if (!window) {
		SDL_Quit();
		return 1;
	}

	if (!AudioSystem::init()) {
		SDL_DestroyWindow(window);
		SDL_Quit();
		return 1;
	}

	MusicTrack test_song("assets/audio/test.ogg");
	test_song.play();

	SoundEffect test_sound("assets/audio/test.wav");
	test_sound.play(-1); // loop indefinitely (testing loops)

	SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
	if (!renderer) {
		SDL_DestroyWindow(window);
		SDL_Quit();
		return 1;
	}

	Spritesheet sheet(renderer, "assets/sprites/arcanist.png", 24, 24);
	Tilemap map(&sheet, 24, 24, 10, 10, "assets/maps/test_map.txt");
	InputHandler handler;

	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);

	while (!quit) {
		SDL_WaitEvent(&event);

		switch (event.type) {
		case SDL_QUIT:
			quit = true;
			break;
		case SDL_KEYDOWN:
			handler.handle_keydown(event.key.keysym.sym);
			break;
		case SDL_KEYUP:
			handler.handle_keyup(event.key.keysym.sym);
			break;
		case SDL_MOUSEMOTION:
			handler.handle_mousemotion(event.motion.x, event.motion.y);
			break;
		case SDL_MOUSEBUTTONDOWN:
			handler.handle_mousebuttondown(event.button.button, event.button.x, event.button.y);
			break;
		case SDL_MOUSEBUTTONUP:
			handler.handle_mousebuttonup(event.button.button, event.button.x, event.button.y);
			break;
		}

		map.draw(renderer, 0, 0, 24, 24);

		sheet.select_sprite(0);
		sheet.draw(renderer, 100, 100, 96, 96);
		SDL_RenderPresent(renderer);

		SDL_RenderClear(renderer);
	}

	SDL_DestroyWindow(window);
	SDL_Quit();
	return 0;
}
