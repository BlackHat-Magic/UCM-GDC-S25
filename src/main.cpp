#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <algorithm>
#include <limits>

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>

#include "utils/spritesheet.h"
#include "utils/audio.h"
#include "utils/tilemap.h"
#include "utils/input.h"

#include "game/player.h"
#include "game/geezer.h"
#include "game/entity_manager.h"

enum class GameState {
	MAIN_MENU,
	PLAYING,
	PAUSED,
	GAME_OVER,
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
		std::cerr << "Menu font could not initialize: " << TTF_GetError () << std::endl;
		AudioSystem::quit ();
		SDL_DestroyRenderer (renderer);
		SDL_DestroyWindow (window);
		TTF_Quit ();
		IMG_Quit ();
		SDL_Quit ();
		return 1;
	}

	TTF_Font* hudFont = TTF_OpenFont ("assets/fonts/press_start/prstart.ttf", 14);
	if (!hudFont) {
		std::cerr << "HUD font could not initialize: " << TTF_GetError () << std:: endl;
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
	SDL_Texture* desktopTexture = renderText (renderer, menuFont, "Quit to Desktop", white);
	SDL_Texture* startTextureHover = renderText (renderer, menuFont, "Start Game", gray);
	SDL_Texture* desktopTextureHover = renderText (renderer, menuFont, "Quit to Desktop", gray);
	// texture dimensions for positioning
	int startW, startH, desktopW, desktopH;
	SDL_QueryTexture (startTexture, NULL, NULL, &startW, &startH);
	SDL_QueryTexture (desktopTexture, NULL, NULL, &desktopW, &desktopH);
	SDL_Rect startButtonRect = {640 / 2 - startW / 2, 200, startW, startH};
	SDL_Rect desktopButtonRect = {640 / 2 - desktopW / 2, 260, desktopW, desktopH};

	// pause menu
	SDL_Texture* continueTexture = renderText (renderer, menuFont, "Continue", white);
	SDL_Texture* menuTexture = renderText (renderer, menuFont, "Quit to Menu", white);
	SDL_Texture* continueTextureHover = renderText (renderer, menuFont, "Continue", gray);
	SDL_Texture* menuTextureHover = renderText (renderer, menuFont, "Quit to Menu", gray);
	// desktop button is reused
	int continueW, continueH, menuW, menuH;
	SDL_QueryTexture (continueTexture, NULL, NULL, &continueW, &continueH);
	SDL_QueryTexture (menuTexture, NULL, NULL, &menuW, &menuH);
	SDL_Rect continueButtonRect = {640 / 2 - continueW / 2, 140, continueW, continueH};
	SDL_Rect menuButtonRect = {640 / 2 - menuW / 2, 200, menuW, menuH};
	// desktop button is reused

	// game over
	SDL_Texture* gameOverTexture = renderText (renderer, menuFont, "GAME OVER", {255, 0, 0, 255});
	SDL_Texture* restartTexture = renderText (renderer, menuFont, "Restart", white);
	SDL_Texture* restartTextureHover = renderText (renderer, menuFont, "Restart", gray);
	// quit to menu, desktop reused from above
	int gameOverW, gameOverH, restartW, restartH;
	SDL_QueryTexture (gameOverTexture, NULL, NULL, &gameOverW, &gameOverH);
	SDL_QueryTexture (restartTexture, NULL, NULL, &restartW, &restartH);
	SDL_Rect gameOverRect = {640 / 2 - gameOverW / 2, 80, gameOverW, gameOverH};
	SDL_Rect restartButtonRect = {640 / 2 - restartW / 2, 140, restartW, restartH};

	// menu background texture eventually
	// SDL_Texture* menuBackground = IMG_LoadTexture (renderer, "assets/ui/menu_background.png");

	EntityManager entityManager (renderer);
	entityManager.setScreenDimensions (640, 480);

	// load music, initialzie input handler
	MusicTrack menu_music("assets/audio/patient_rituals.mp3");
	MusicTrack level_music("assets/audio/level_theme.mp3");
	InputHandler handler;

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
	menu_music.setVolume(10);

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
				} else if (currentState == GameState::MAIN_MENU || currentState == GameState::GAME_OVER) {
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

	SDL_Point mousePoint = {mouseX, mouseY};
	bool onStart = SDL_PointInRect (&mousePoint, &startButtonRect);
	bool onDesktop = SDL_PointInRect (&mousePoint, &desktopButtonRect);
	bool onContinue = SDL_PointInRect (&mousePoint, &continueButtonRect);
	bool onMenu = SDL_PointInRect (&mousePoint, &menuButtonRect);
	bool onRestart = SDL_PointInRect (&mousePoint, &restartButtonRect);


	SDL_Rect overlayRect = {0, 0, 640, 480};

	switch (currentState) {
	// main menu case
	case GameState::MAIN_MENU: {
		// click main menu buttons
		if (mousePressed) {
			if (onStart) {
				currentState = GameState::PLAYING;
				menu_music.pause ();
				level_music.play (-1);
				level_music.setVolume (50);

				entityManager.clearAll ();

				Player* playerPtr = entityManager.addEntity<Player>(
					renderer, 
					&handler, 
					320, 400, 
					100.0f
				);

				// Define Geezer animations (can be loaded from config later)
				int* geezer_idle = new int[2]{0, -1};
				int* geezer_walk = new int[2]{0, -1};
				int* geezer_atk = new int[2]{0, -1};
				int** geezerAnimations = new int*[4];
				geezerAnimations[0] = geezer_idle;
				geezerAnimations[1] = geezer_walk;
				geezerAnimations[2] = geezer_atk;
				geezerAnimations[3] = nullptr;
				// TODO: Geezer Animations
				if (playerPtr) {
					entityManager.addEntity<Geezer>(
						renderer,
						&entityManager,
						"assets/sprites/geezer.png",
						24, 24,
						320, 100,
						geezerAnimations,
						0.15f, 120.0f,
						playerPtr
					);
				} else {
					std::cerr << "Player failed to initialize." << std::endl;
					currentState = GameState::MAIN_MENU;
					level_music.stop ();
					menu_music.play (-1);
				}

				mousePressed = false;
			} else if (onDesktop) {
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
		SDL_RenderCopy (renderer, onDesktop ? desktopTextureHover : desktopTexture, NULL, &desktopButtonRect);
	} break;

	case GameState::PAUSED: {
		surface_map.draw (renderer, 0, 0);
		trapdoor_map.draw (renderer, 0, 0);
		entityManager.render ();

		SDL_SetRenderDrawBlendMode (renderer, SDL_BLENDMODE_BLEND);
		SDL_SetRenderDrawColor (renderer, 0, 0, 0, 128);
		SDL_RenderFillRect (renderer, &overlayRect);
		SDL_SetRenderDrawBlendMode (renderer, SDL_BLENDMODE_NONE);
		SDL_SetRenderDrawColor (renderer, 0, 0, 0, 255);

		// click pause buttons
		if (mousePressed) {
			if (onContinue) {
				currentState = GameState::PLAYING;
				level_music.setVolume (50);
				mousePressed = false;
			} else if (onMenu) {
				currentState = GameState::MAIN_MENU;
				entityManager.clearAll ();
				level_music.stop ();
				menu_music.play(-1);
				mousePressed = false;
			} else if (onDesktop) {
				gameRunning = false;
			}
		}

		SDL_RenderCopy (renderer, onContinue ? continueTextureHover : continueTexture, NULL, &continueButtonRect);
		SDL_RenderCopy (renderer, onMenu ? menuTextureHover : menuTexture, NULL, &menuButtonRect);
		SDL_RenderCopy (renderer, onDesktop ? desktopTextureHover : desktopTexture, NULL, &desktopButtonRect);
	} break;

	case GameState::PLAYING: {
		entityManager.update (&trapdoor_map, time, deltaTime);
		entityManager.cleanupEntities ();

		Player* player= entityManager.getPlayer ();
		if (!player || !player->isAlive ()) {
			currentState = GameState::GAME_OVER;
			level_music.stop ();

			break;
		}

		// render game
		surface_map.draw (renderer, 0, 0);
		trapdoor_map.draw (renderer, 0, 0);
		entityManager.render ();

		// render HUD
		std::stringstream healthText;
		healthText << "Health: " << static_cast<int>(player->getHealth()) << " / " << static_cast<int>(player->getMaxHealth());
		SDL_Texture* healthTexture = renderText (renderer, hudFont, healthText.str(), white);
		if (healthTexture) {
			int healthW, healthH;
			SDL_QueryTexture (healthTexture, NULL, NULL, &healthW, &healthH);
			SDL_Rect healthRect = {10, 10, healthW, healthH};
			SDL_RenderCopy (renderer, healthTexture, NULL, &healthRect);
			SDL_DestroyTexture (healthTexture);
		}
	} break;

	case GameState::GAME_OVER: {
		surface_map.draw (renderer, 0, 0);
		trapdoor_map.draw (renderer, 0, 0);
		entityManager.render ();

		SDL_SetRenderDrawBlendMode (renderer, SDL_BLENDMODE_BLEND);
		SDL_SetRenderDrawColor (renderer, 20, 0, 0, 160);
		SDL_RenderFillRect (renderer, &overlayRect);
		SDL_SetRenderDrawBlendMode (renderer, SDL_BLENDMODE_NONE);
		SDL_SetRenderDrawColor (renderer, 0, 0, 0, 255);

		if (mousePressed) {
			if (onRestart) {
				currentState = GameState::PLAYING;
				level_music.play(-1);
				level_music.setVolume (50);

				entityManager.clearAll ();

				Player* playerPtr = entityManager.addEntity<Player>(
					renderer, 
					&handler, 
					320, 400, 
					100.0f
				);

				// Define Geezer animations (can be loaded from config later)
				int* geezer_idle = new int[2]{0, -1};
				int* geezer_walk = new int[2]{0, -1};
				int* geezer_atk = new int[2]{0, -1};
				int** geezerAnimations = new int*[4];
				geezerAnimations[0] = geezer_idle;
				geezerAnimations[1] = geezer_walk;
				geezerAnimations[2] = geezer_atk;
				geezerAnimations[3] = nullptr;
				// TODO: Geezer Animations
				if (playerPtr) {
					entityManager.addEntity<Geezer>(
						renderer,
						&entityManager,
						"assets/sprites/geezer.png",
						24, 24,
						320, 100,
						geezerAnimations,
						0.15f, 120.0f,
						playerPtr
					);
				} else {
					std::cerr << "Player failed to initialize." << std::endl;
					currentState = GameState::MAIN_MENU;
					level_music.stop ();
					menu_music.play (-1);
				}
			} else if (onMenu) {
				currentState = GameState::MAIN_MENU;
				entityManager.clearAll ();
				menu_music.play(-1);
				mousePressed = false;
			} else if (onDesktop) {
				gameRunning = false;
			}
		}
		SDL_RenderCopy (renderer, gameOverTexture, NULL, &gameOverRect);
		SDL_RenderCopy (renderer, onRestart ? restartTextureHover : restartTexture, NULL, &restartButtonRect);
		SDL_RenderCopy (renderer, onMenu ? menuTextureHover : menuTexture, NULL, &menuButtonRect);
		SDL_RenderCopy (renderer, onDesktop ? desktopTextureHover : desktopTexture, NULL, &desktopButtonRect);
	} break;

	case GameState::QUIT:
		gameRunning = false;
		break;
	}

	SDL_RenderPresent (renderer);
}

entityManager.clearAll ();
delete[] collidables;

SDL_DestroyTexture(startTexture);
SDL_DestroyTexture(startTextureHover);
SDL_DestroyTexture(desktopTexture);
SDL_DestroyTexture(desktopTextureHover);

SDL_DestroyTexture(continueTexture);
SDL_DestroyTexture(continueTextureHover);
SDL_DestroyTexture(menuTexture);
SDL_DestroyTexture(menuTextureHover);

SDL_DestroyTexture (gameOverTexture);
SDL_DestroyTexture (restartTexture);

TTF_CloseFont(menuFont);
TTF_CloseFont(hudFont);
AudioSystem::quit ();
SDL_DestroyRenderer (renderer);
SDL_DestroyWindow(window);
TTF_Quit ();
IMG_Quit ();
SDL_Quit();
return 0;
}
