#include <SDL2/SDL.h>

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

	SDL_Delay(2000);

	SDL_DestroyWindow(window);
	SDL_Quit();
	return 0;
}
