#include <SDL2/SDL.h>
#include "utils/spritesheet.h"
#include "utils/audio.h"
#include "utils/tilemap.h"
#include "utils/input.h"
#include <iostream>
#include "character.h"

int main() {

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
	Character character (renderer, "assets/sprites/arcanist.png", 24, 24, 100, 100);

	// Tilemap map(&sheet, 24, 24, 10, 10, "assets/maps/test_map.txt");

	bool quit = false;
	SDL_Event event;
	Uint32 lastTime = SDL_GetTicks ();

	while (!quit) {
		while (SDL_PollEvent (&event)) {
			if (event.type == SDL_QUIT) {
				quit = true;
			}
			character.handleEvent (event);
		}

		// delta time
		Uint32 currentTime = SDL_GetTicks ();
		float deltaTime = (currentTime - lastTime) / 1000.0f;
		lastTime = currentTime;
		
		character.update (deltaTime);

		SDL_SetRenderDrawColor (renderer, 0, 0, 0, 255);
		SDL_RenderClear (renderer);
		character.render (renderer);
		SDL_RenderPresent (renderer);

		SDL_Delay (16);
	}

	SDL_DestroyWindow(window);
	SDL_Quit();
	return 0;
}
