#include <SDL2/SDL.h>
#include "utils/spritesheet.h"
#include "utils/audio.h"

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
	sheet.select_sprite(0, 0);
	sheet.draw(renderer, 100, 100);
	SDL_RenderPresent(renderer);

	SDL_Delay(300);

	sheet.select_sprite(1, 0);
	sheet.draw(renderer, 100, 100);
	SDL_RenderPresent(renderer);

	SDL_Delay(300);

	sheet.select_sprite(2, 0);
	sheet.draw(renderer, 100, 100);
	SDL_RenderPresent(renderer);

	SDL_Delay(10000);

	SDL_DestroyWindow(window);
	SDL_Quit();
	return 0;
}
