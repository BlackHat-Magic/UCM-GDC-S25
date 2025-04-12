#include <SDL2/SDL.h>
#include "utils/spritesheet.h"
#include "utils/audio.h"
#include "utils/tilemap.h"
#include "utils/input.h"
#include <iostream>
#include "game/player.h"

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
	Player player (renderer, &handler, 100, 100);

	// Tilemap map(&sheet, 24, 24, 10, 10, "assets/maps/test_map.txt");

	bool quit = false;
	SDL_Event event;
	float lastTime = SDL_GetTicks () / 1000.0f;

	SDL_SetRenderDrawColor (renderer, 0, 0, 0, 255);

	while (!quit) {
		while (SDL_PollEvent (&event)) {
			switch (event.type) {
				case SDL_QUIT:
					quit = true;
					break;
				case SDL_KEYDOWN:
					if (!event.key.repeat)
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
		}

		Uint32 currentTime = SDL_GetTicks ();
		float time = currentTime / 1000.0f;
		float deltaTime = time - lastTime;
		lastTime = time;
		
		player.update (time, deltaTime);

		SDL_RenderClear (renderer);
		player.render (renderer);
		SDL_RenderPresent (renderer);

		SDL_Delay (16);
	}

	SDL_DestroyWindow(window);
	SDL_Quit();
	return 0;
}
