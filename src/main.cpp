#include <SDL2/SDL.h>
#include "utils/spritesheet.h"
#include "utils/audio.h"
#include "utils/tilemap.h"
#include "utils/input.h"
#include <iostream>
#include "game/player.h"
#include "game/geezer.h"

int main() {
	// initialize SDL
	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		return 1;
	}

	// spawn window
	SDL_Window* window = SDL_CreateWindow("Hello SDL2",
										  SDL_WINDOWPOS_UNDEFINED,
										  SDL_WINDOWPOS_UNDEFINED,
										  640, 480,
										  SDL_WINDOW_SHOWN);
	if (!window) {
		SDL_Quit();
		return 1;
	}

	// initialize audio system
	if (!AudioSystem::init()) {
		SDL_DestroyWindow(window);
		SDL_Quit();
		return 1;
	}

	// load music
	MusicTrack music("assets/audio/Patient Rituals.mp3");
	music.play(-1);
	music.setVolume(50);

	// initialize renderer
	SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
	if (!renderer) {
		SDL_DestroyWindow(window);
		SDL_Quit();
		return 1;
	}

	// spritesheet, tilemap, inputhandler, player
	Spritesheet sheet(renderer, "assets/tilesets/kenney_tiny-dungeon/tilemap_packed.png", 16, 16);
	Tilemap map(&sheet, 16, 16, 80, 30, new int[2] {0, -1 } ,"assets/maps/test_map.txt");
	InputHandler handler;

	// create player
	Player player (renderer, &handler, 300, 300);

	// create geezer animations
	int* idleAnimation = new int[2]{0, -1};
	int** geezerAnimations = new int*[3];
	geezerAnimations[0] = idleAnimation;
	geezerAnimations[1] = idleAnimation;
	geezerAnimations[2] = idleAnimation;

	// create geezer
	Geezer geezer (
		renderer,
		"assets/sprites/geezer.png",
		24, 24,
		300, 150,
		geezerAnimations,
		0.1f,
		200.0f,
		&player
	);

	// loop upkeep stuff
	bool quit = false;
	SDL_Event event;
	float lastTime = SDL_GetTicks () / 1000.0f;

	// camera setup
	float cameraX = 0.0f;
	float cameraY = 0.0f;
	const float cameraScrollSpeed = 16.0f;

	SDL_SetRenderDrawColor (renderer, 0, 0, 0, 255);
	
	// game loop
	while (!quit) {
		// handle events (e.g., movement)
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

		// time stuff
		Uint32 currentTime = SDL_GetTicks ();
		float time = currentTime / 1000.0f;
		float deltaTime = time - lastTime;
		lastTime = time;

		// camera scroll
		cameraX += cameraScrollSpeed * deltaTime;
		
		// update player
		player.update (&map, time, deltaTime, cameraX, cameraY);

		// update geezer
		geezer.update (&map, time, deltaTime, cameraX, cameraY);

		// clear renderer
		SDL_RenderClear (renderer);

		// render with camera offset
		map.draw (renderer, cameraX, cameraY);
		player.render (renderer, cameraX, cameraY);
		geezer.render (renderer, cameraX, cameraY);

		SDL_RenderPresent (renderer);

		// 60(ish) fps
		SDL_Delay (16);
	}

	SDL_DestroyWindow(window);
	SDL_Quit();
	return 0;
}
