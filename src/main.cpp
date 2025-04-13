#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include "utils/spritesheet.h"
#include "utils/audio.h"
#include "utils/tilemap.h"
#include "utils/input.h"
#include <iostream>
#include <vector>
#include <string>
#include "game/player.h"
#include "game/geezer.h"

enum class GameState {
	MAIN_MENU,
	PLAYING,
	PAUSED,
	QUIT
};

SDL_Texture* renderText (SDL_Renderer* renderer, TTF_Font* font, const std::string &text, SDL_Color color) {
	SDL_Surface* surface = TTF_RenderText_Solid (font, text.c_str(), color);
	if (!surface) {
		std::cerr << "Failed to render text surface: " << TTF_GetError () << std::endl;
		return nullptr;
	}

	SDL_Texture* texture = SDL_CreateTextureFromSurface (renderer, surface);
	if (!texture) {
		std::cerr << "Failed to create texture from rendered text: " << SDL_GetError () << std::endl;
	}
	SDL_FreeSurface (surface);
	return (texture);
}

int main(int argc, char* argv[]) {
	// initialize SDL
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
		std::cout << "SDL could not initialize: " << SDL_GetError() << std::endl;
		return 1;
	}

	// initialize SDL_Image
	if (!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG)) {
		std::cerr << "SDL_image could not initialize: " << IMG_GetError () << std::endl;
		return 1;
	}

	// initialize ttf
	if (TTF_Init () == -1) {
		std::cerr << "SDL_ttf could not initialize: " << TTF_GetError () << std::endl;
	}

	// spawn window
	SDL_Window* window = SDL_CreateWindow(
		"Hello SDL2",
		SDL_WINDOWPOS_UNDEFINED,
		SDL_WINDOWPOS_UNDEFINED,
		640, 480,
		SDL_WINDOW_SHOWN
	);
	if (!window) {
		std::cerr << "Window could not initialize: " << SDL_GetError () << std::endl;
		TTF_Quit ();
		IMG_Quit ();
		SDL_Quit();
		return 1;
	}

	// initialize renderer
	SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
	if (!renderer) {
		std::cerr << "Renderer could not initialize: " << SDL_GetError () << std::endl;
		SDL_DestroyWindow(window);
		TTF_Quit ();
		IMG_Quit ();
		SDL_Quit();
		return 1;
	}
	SDL_SetRenderDrawColor (renderer, 0, 0, 0, 255);

	// initialize audio system
	if (!AudioSystem::init()) {
		std::cerr << "Audio system could not initialize." << std::endl;
		SDL_DestroyRenderer (renderer);
		SDL_DestroyWindow(window);
		TTF_Quit ();
		IMG_Quit ();
		SDL_Quit();
		return 1;
	}

	// menu font
	TTF_Font* menuFont = TTF_OpenFont ("assets/fonts/press_start/prstart.ttf", 28);
	if (!menuFont) {
		std::cerr << "Font could not initialize: " << TTF_GetError () << std::endl;
		AudioSystem::quit ();
		SDL_DestroyRenderer (renderer);
		SDL_DestroyWindow (window);
		TTF_Quit ();
		IMG_Quit ();
		SDL_Quit ();
		return 1;
	}

	// main menu resources
	SDL_Color white {255, 255, 255, 255};
	SDL_Color gray = {150, 150, 150, 255};
	SDL_Texture* startTexture = renderText (renderer, menuFont, "Start Game", white);
	SDL_Texture* quitTexture = renderText (renderer, menuFont, "Quit", white);
	SDL_Texture* startTextureHover = renderText (renderer, menuFont, "Start Game", gray);
	SDL_Texture* quitTextureHover = renderText (renderer, menuFont, "Quit", gray);

	// texture dimensions for positioning
	int startW, startH, quitW, quitH;
	SDL_QueryTexture (startTexture, NULL, NULL, &startW, &startH);
	SDL_QueryTexture (quitTexture, NULL, NULL, &quitW, &quitH);
	SDL_Rect startButtonRect = {640 / 2 - startW / 2, 200, startW, startH};
	SDL_Rect quitButtonRect = {640 / 2 - quitW / 2, 260, quitW, quitH};

	// same for pause menu
	SDL_Texture* continueTexture = renderText (renderer, menuFont, "Continue", white);
	SDL_Texture* menuTexture = renderText (renderer, menuFont, "Quit to Menu", white);
	SDL_Texture* desktopTexture = renderText (renderer, menuFont, "Quit to Desktop", white);
	SDL_Texture* continueTextureHover = renderText (renderer, menuFont, "Continue", gray);
	SDL_Texture* menuTextureHover = renderText (renderer, menuFont, "Quit to Menu", gray);
	SDL_Texture* desktopTextureHover = renderText (renderer, menuFont, "Quit to Desktop", gray);

	// agane
	int continueW, continueH, menuW, menuH, desktopW, desktopH;
	SDL_QueryTexture (continueTexture, NULL, NULL, &continueW, &continueH);
	SDL_QueryTexture (menuTexture, NULL, NULL, &menuW, &menuH);
	SDL_QueryTexture (desktopTexture, NULL, NULL, &desktopW, &desktopH);
	SDL_Rect continueButtonRect = {640 / 2 - continueW / 2, 140, continueW, continueH};
	SDL_Rect menuButtonRect = {640 / 2 - menuW / 2, 200, menuW, menuH};
	SDL_Rect desktopButtonRect = {640 / 2 - desktopW / 2, 260, desktopW, desktopH};

	// menu background texture eventually
	// SDL_Texture* menuBackground = IMG_LoadTexture (renderer, "assets/ui/menu_background.png");

	// load music, initialzie input handler
	MusicTrack menu_music("assets/audio/patient_rituals.mp3");
	MusicTrack level_music("assets/audio/level_theme.mp3");
	InputHandler handler;

	// entities are initialized as empty
	Player* player = nullptr;
	Geezer* geezer = nullptr;
	int** geezerAnimations = nullptr;

	// load tileset and map once
	Spritesheet sheet(renderer, "assets/tilesets/Dungeon_16x16_asset_pack/tileset.png", 16, 16);
    int* collidables = new int[14]{0, 1, 2, 3, 4, 5, 6, 7, 8, 12, 13, 14, 26, -1}; // Manage this memory
    Tilemap surface_map(&sheet, 16, 16, 6, 7, collidables, "assets/maps/test_map_surfaces.txt");
    Tilemap trapdoor_map(&sheet, 16, 16, 6, 7, collidables, "assets/maps/test_map_trapdoors.txt");

	// Game Loop Variables
	GameState currentState = GameState::MAIN_MENU;
	bool gameRunning = true;
	SDL_Event event;
	float lastTime = SDL_GetTicks () / 1000.0f;
	int mouseX = 0, mouseY = 0;
	bool mousePressed = false;
	
	menu_music.play(-1);
	menu_music.setVolume(50);

	// game loop
	while (gameRunning) {
	// handle events (e.g., movement)
	mousePressed = false;
	while (SDL_PollEvent (&event)) {
		switch (event.type) {
		case SDL_QUIT:
			gameRunning = false;
			break;
		case SDL_KEYDOWN:
			if (!event.key.repeat) {
				handler.handle_keydown (event.key.keysym.sym);
			}
			// escape key
			if (event.key.keysym.sym == SDLK_ESCAPE) {
				if (currentState == GameState::PLAYING) {
					currentState = GameState::PAUSED;
					level_music.setVolume (10);
				} else if (currentState == GameState::MAIN_MENU) {
					gameRunning = false;
				} else if (currentState == GameState::PAUSED) {
					currentState = GameState::PLAYING;
					level_music.setVolume (50);
				}
			}
			break;
		case SDL_KEYUP:
			handler.handle_keyup(event.key.keysym.sym);
			break;
		case SDL_MOUSEMOTION:
			handler.handle_mousemotion(event.motion.x, event.motion.y);
			mouseX = event.motion.x;
			mouseY = event.motion.y;
			break;
		case SDL_MOUSEBUTTONDOWN:
			handler.handle_mousebuttondown(event.button.button, event.button.x, event.button.y);
			if (event.button.button == SDL_BUTTON_LEFT) {
				mousePressed = true;
			}
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
	if (deltaTime > 0.1f) deltaTime = 0.1f;

	SDL_RenderClear (renderer);

	switch (currentState) {
	// main menu case
	case GameState::MAIN_MENU: {
		SDL_Point mousePoint = {mouseX, mouseY};
		bool onStart = SDL_PointInRect (&mousePoint, &startButtonRect);
		bool onQuit = SDL_PointInRect (&mousePoint, &quitButtonRect);

		// click main menu buttons
		if (mousePressed) {
			if (onStart) {
				currentState = GameState::PLAYING;
				menu_music.pause ();
				level_music.play (-1);
				level_music.setVolume (50);

				player = new Player (renderer, &handler, 300, 300);

				// Create geezer animations
				geezerAnimations = new int*[4];
				geezerAnimations[0] = new int[2]{0, -1};
				geezerAnimations[1] = new int[2]{0, -1};
				geezerAnimations[2] = new int[2]{0, -1};
				geezerAnimations[3] = nullptr;
				// TODO: Geezer Animations

				geezer = new Geezer (renderer, "assets/sprites/geezer.png", 24, 24, 300, 150, geezerAnimations, 0.1f, 150.0f, player);
				mousePressed = false;
			} else if (onQuit) {
				gameRunning = false;
			}
		}

		// menu background someday
		// SDL_RenderCopy (renderer, menuBackground, NULL, NULL);

		// color for now :pensive:
		SDL_SetRenderDrawColor (renderer, 50, 20, 10, 255);
		SDL_RenderFillRect (renderer, NULL);
		SDL_SetRenderDrawColor (renderer, 0, 0, 0, 255);

		SDL_RenderCopy (renderer, onStart ? startTextureHover : startTexture, NULL, &startButtonRect);
		SDL_RenderCopy (renderer, onQuit ? quitTextureHover : quitTexture, NULL, &quitButtonRect);
	} break;

	case GameState::PAUSED: {
		if (player && geezer) {
			surface_map.draw (renderer, 0, 0);
			trapdoor_map.draw (renderer, 0, 0);
			player->render(renderer);
			geezer->render(renderer);
		}

		SDL_SetRenderDrawBlendMode (renderer, SDL_BLENDMODE_BLEND);
		SDL_SetRenderDrawColor (renderer, 0, 0, 0, 150);
		SDL_Rect overlayRect = {0, 0, 640, 480};
		SDL_RenderFillRect (renderer, &overlayRect);
		SDL_SetRenderDrawBlendMode (renderer, SDL_BLENDMODE_NONE);
		SDL_SetRenderDrawColor (renderer, 0, 0, 0, 255);
		SDL_Point mousePoint = {mouseX, mouseY};
		bool onContinue = SDL_PointInRect (&mousePoint, &continueButtonRect);
		bool onMenu = SDL_PointInRect (&mousePoint, &menuButtonRect);
		bool onDesktop = SDL_PointInRect (&mousePoint, &desktopButtonRect);

		// click pause buttons
		if (mousePressed) {
			if (onContinue) {
				currentState = GameState::PLAYING;
				level_music.setVolume (50);
				mousePressed = false;
			} else if (onMenu) {
				currentState = GameState::MAIN_MENU;
				delete player; player = nullptr;
				delete geezer; geezer = nullptr;
				mousePressed = false;
				level_music.pause();
				menu_music.play(-1);
			} else if (onDesktop) {
				gameRunning = false;
			}
		}

		SDL_RenderCopy (renderer, onContinue ? continueTextureHover : continueTexture, NULL, &continueButtonRect);
		SDL_RenderCopy (renderer, onMenu ? menuTextureHover : menuTexture, NULL, &menuButtonRect);
		SDL_RenderCopy (renderer, onDesktop ? desktopTextureHover : desktopTexture, NULL, &desktopButtonRect);
	} break;

	case GameState::PLAYING: {
		if (!player) {
			std::cerr << "Player failed to initialize." << std::endl;
			currentState = GameState::MAIN_MENU;
			break;
		}
		if (!geezer) {
			std::cerr << "Geezer failed to initialize." << std::endl;
			currentState = GameState::MAIN_MENU;
			break;
		}

		player->update (&trapdoor_map, time, deltaTime);
		geezer->update (&trapdoor_map, time, deltaTime);

		surface_map.draw (renderer, 0, 0);
		trapdoor_map.draw (renderer, 0, 0);
		player->render (renderer);
		geezer-> render (renderer);
	} break;

	case GameState::QUIT:
		gameRunning = false;
		break;
	}

	SDL_RenderPresent (renderer);
}

delete player;
delete geezer;
delete[] collidables;

SDL_DestroyTexture(startTexture);
SDL_DestroyTexture(startTextureHover);
SDL_DestroyTexture(quitTexture);
SDL_DestroyTexture(quitTextureHover);

SDL_DestroyTexture(continueTexture);
SDL_DestroyTexture(continueTextureHover);
SDL_DestroyTexture(menuTexture);
SDL_DestroyTexture(menuTextureHover);
SDL_DestroyTexture(desktopTexture);
SDL_DestroyTexture(desktopTextureHover);

AudioSystem::quit ();
SDL_DestroyRenderer (renderer);
SDL_DestroyWindow(window);
TTF_Quit ();
IMG_Quit ();
SDL_Quit();
return 0;
}
