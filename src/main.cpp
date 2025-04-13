// src/main.cpp
#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <algorithm>
#include <limits>
#include <map> // Include map for tile layers
#include <memory> // For shared_ptr

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>

#include "utils/spritesheet.h"
#include "utils/audio.h"
#include "utils/tilemap.h"
#include "utils/input.h"
#include "utils/collisions_defs.h" // Include collision definitions
#include "utils/tmx_parser.h" // Include our new TMX parser

#include "game/player.h"
#include "game/geezer.h"
#include "game/entity_manager.h"

enum class GameState { MAIN_MENU, PLAYING, PAUSED, GAME_OVER, QUIT };

// Helper function to render text
SDL_Texture* renderText(
    SDL_Renderer* renderer, TTF_Font* font, const std::string& text,
    SDL_Color color
) {
    SDL_Surface* surface = TTF_RenderText_Solid(font, text.c_str(), color);
    if (!surface) {
        std::cerr << "Failed to render text surface: " << TTF_GetError()
                  << std::endl;
        return nullptr;
    }

    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    if (!texture) {
        std::cerr << "Failed to create texture from rendered text: "
                  << SDL_GetError() << std::endl;
    }
    SDL_FreeSurface(surface);
    return texture;
}

// Function to set up entities for a new game/restart
void setupNewGame(EntityManager& entityManager, SDL_Renderer* renderer, InputHandler& handler) {
     entityManager.clearAll();

     // Create Player
     Player* playerPtr = entityManager.addEntity<Player>(
         renderer, &handler, 320.0f, 400.0f, // Initial position (use floats)
         100.0f                             // Initial health
     );

     if (!playerPtr) {
         std::cerr << "FATAL: Player failed to initialize." << std::endl;
         // Handle this critical error (e.g., throw exception, exit)
         return; // Or return bool success status
     }

     // --- Define Geezer animations ---
     // IMPORTANT: Replace these placeholders with actual sprite indices!
     int* geezer_idle = new int[2]{0, -1}; // Sprite index 0 for idle
     int* geezer_walk = new int[2]{0, -1}; // Sprite index 0 for walk (NEEDS FIXING)
     int* geezer_atk = new int[2]{0, -1};  // Sprite index 0 for attack (NEEDS FIXING)
     int** geezerAnimations = new int*[4];
     geezerAnimations[0] = geezer_idle;
     geezerAnimations[1] = geezer_walk;
     geezerAnimations[2] = geezer_atk;
     geezerAnimations[3] = nullptr; // Terminator

     // Create Geezer targeting the player
     entityManager.addEntity<Geezer>(
         renderer,
         &entityManager, // Pass EntityManager for fireball spawning
         "assets/sprites/geezer.png",
         24, 24,          // Sprite dimensions
         320.0f, 100.0f,  // Initial position
         geezerAnimations, // The defined animations
         0.15f,           // Animation speed
         120.0f,          // Movement speed
         playerPtr        // Target
     );

     // Add more enemies or other entities here
}


int main(int argc, char* argv[]) {
    // --- SDL Initialization ---
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
        std::cerr << "SDL could not initialize: " << SDL_GetError() << std::endl;
        return 1;
    }
    if (!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG)) {
        std::cerr << "SDL_image could not initialize: " << IMG_GetError()
                  << std::endl;
        SDL_Quit();
        return 1;
    }
    if (TTF_Init() == -1) {
        std::cerr << "SDL_ttf could not initialize: " << TTF_GetError()
                  << std::endl;
        IMG_Quit();
        SDL_Quit();
        return 1;
    }
    if (!AudioSystem::init()) {
        std::cerr << "Audio system could not initialize." << std::endl;
        TTF_Quit();
        IMG_Quit();
        SDL_Quit();
        return 1;
    }

    // --- Window & Renderer ---
    SDL_Window* window = SDL_CreateWindow(
        "Game Title", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 640, 480,
        SDL_WINDOW_SHOWN
    );
    if (!window) { /* ... error handling ... */ return 1; }
    SDL_Renderer* renderer = SDL_CreateRenderer(
        window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC
    );
    if (!renderer) { /* ... error handling ... */ return 1; }
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); // Default background

    // --- Fonts ---
    TTF_Font* menuFont = TTF_OpenFont("assets/fonts/press_start/prstart.ttf", 28);
    TTF_Font* hudFont = TTF_OpenFont("assets/fonts/press_start/prstart.ttf", 14);
    if (!menuFont || !hudFont) { /* ... error handling ... */ return 1; }

    // --- Menu Resources ---
    SDL_Color white = {255, 255, 255, 255};
    SDL_Color gray = {150, 150, 150, 255};
    SDL_Color red = {255, 0, 0, 255};
    // Main Menu
    SDL_Texture* startTexture = renderText(renderer, menuFont, "Start Game", white);
    SDL_Texture* desktopTexture = renderText(renderer, menuFont, "Quit to Desktop", white);
    SDL_Texture* startTextureHover = renderText(renderer, menuFont, "Start Game", gray);
    SDL_Texture* desktopTextureHover = renderText(renderer, menuFont, "Quit to Desktop", gray);
    int startW, startH, desktopW, desktopH;
    SDL_QueryTexture(startTexture, NULL, NULL, &startW, &startH);
    SDL_QueryTexture(desktopTexture, NULL, NULL, &desktopW, &desktopH);
    SDL_Rect startButtonRect = {320 - startW / 2, 200, startW, startH};
    SDL_Rect desktopButtonRect = {320 - desktopW / 2, 260, desktopW, desktopH};
    // Pause Menu
    SDL_Texture* continueTexture = renderText(renderer, menuFont, "Continue", white);
    SDL_Texture* menuTexture = renderText(renderer, menuFont, "Quit to Menu", white);
    SDL_Texture* continueTextureHover = renderText(renderer, menuFont, "Continue", gray);
    SDL_Texture* menuTextureHover = renderText(renderer, menuFont, "Quit to Menu", gray);
    int continueW, continueH, menuW, menuH;
    SDL_QueryTexture(continueTexture, NULL, NULL, &continueW, &continueH);
    SDL_QueryTexture(menuTexture, NULL, NULL, &menuW, &menuH);
    SDL_Rect continueButtonRect = {320 - continueW / 2, 140, continueW, continueH};
    SDL_Rect menuButtonRect = {320 - menuW / 2, 200, menuW, menuH};
    // Game Over Menu
    SDL_Texture* gameOverTexture = renderText(renderer, menuFont, "GAME OVER", red);
    SDL_Texture* restartTexture = renderText(renderer, menuFont, "Restart", white);
    SDL_Texture* restartTextureHover = renderText(renderer, menuFont, "Restart", gray);
    int gameOverW, gameOverH, restartW, restartH;
    SDL_QueryTexture(gameOverTexture, NULL, NULL, &gameOverW, &gameOverH);
    SDL_QueryTexture(restartTexture, NULL, NULL, &restartW, &restartH);
    SDL_Rect gameOverRect = {320 - gameOverW / 2, 80, gameOverW, gameOverH};
    SDL_Rect restartButtonRect = {320 - restartW / 2, 140, restartW, restartH};
    // Shared button rects for pause/game over
    SDL_Rect pauseMenuButtonRect = menuButtonRect; // Reposition if needed
    SDL_Rect pauseDesktopButtonRect = desktopButtonRect;
    SDL_Rect gameOverMenuButtonRect = {320 - menuW / 2, 200, menuW, menuH}; // Same pos as pause
    SDL_Rect gameOverDesktopButtonRect = {320 - desktopW / 2, 260, desktopW, desktopH}; // Same pos as main


    // --- Game Systems ---
    EntityManager entityManager(renderer);
    entityManager.setScreenDimensions(640, 480);
    InputHandler handler;
    MusicTrack menu_music("assets/audio/patient_rituals.mp3");
    MusicTrack level_music("assets/audio/level_theme.mp3");

    // --- TMX Parser and Tilemaps ---
    TmxParser tmxParser;
    std::vector<std::shared_ptr<Tilemap>> tilemaps;
    
    // Load the TMX file
    if (!tmxParser.loadTmx("assets/maps/Demo.tmx", renderer)) {
        std::cerr << "Failed to load TMX file" << std::endl;
        return 1;
    }
    
    // Create tilemaps from the TMX data
    tilemaps = tmxParser.createTilemaps(renderer);
    
    // Get the collision tilemap (assuming the last one has all collision data)
    Tilemap* collisionTilemap = tilemaps.empty() ? nullptr : tilemaps.back().get();

    // --- Game Loop Variables ---
    GameState currentState = GameState::MAIN_MENU;
    bool gameRunning = true;
    SDL_Event event;
    Uint32 lastTick = SDL_GetTicks(); // Use Uint32 for ticks
    int mouseX = 0, mouseY = 0;
    bool mousePressed = false;

    menu_music.play(-1);
    menu_music.setVolume(10); // Low volume for menu

    // --- Main Game Loop ---
    while (gameRunning) {
        // --- Time Calculation ---
        Uint32 currentTick = SDL_GetTicks();
        // Delta time in seconds (ensure float division)
        float deltaTime = (currentTick - lastTick) / 1000.0f;
        // Clamp delta time to avoid large jumps (e.g., during debugging)
        if (deltaTime > 0.1f) deltaTime = 0.1f;
        lastTick = currentTick;
        float time = currentTick / 1000.0f; // Total time in seconds

        // --- Event Handling ---
        mousePressed = false; // Reset mouse press state each frame
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
            case SDL_QUIT:
                gameRunning = false;
                break;
            case SDL_KEYDOWN:
                if (!event.key.repeat) {
                    handler.handle_keydown(event.key.keysym.sym);
                    // Pause/Resume Toggle
                    if (event.key.keysym.sym == SDLK_ESCAPE) {
                        if (currentState == GameState::PLAYING) {
                            currentState = GameState::PAUSED;
                            level_music.setVolume(10); // Lower volume when paused
                        } else if (currentState == GameState::PAUSED) {
                            currentState = GameState::PLAYING;
                            level_music.setVolume(50); // Restore volume
                        } else if (currentState == GameState::MAIN_MENU || currentState == GameState::GAME_OVER) {
                             gameRunning = false; // Esc quits from main/game over
                        }
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
                handler.handle_mousebuttondown(
                    event.button.button, event.button.x, event.button.y
                );
                if (event.button.button == SDL_BUTTON_LEFT) {
                    mousePressed = true;
                }
                break;
            case SDL_MOUSEBUTTONUP:
                handler.handle_mousebuttonup(
                    event.button.button, event.button.x, event.button.y
                );
                break;
            }
        }

        // --- Update Game State ---
        SDL_Point mousePoint = {mouseX, mouseY};

        switch (currentState) {
        case GameState::MAIN_MENU: {
            bool onStart = SDL_PointInRect(&mousePoint, &startButtonRect);
            bool onDesktop = SDL_PointInRect(&mousePoint, &desktopButtonRect);

            if (mousePressed) {
                if (onStart) {
                    currentState = GameState::PLAYING;
                    setupNewGame(entityManager, renderer, handler); // Setup entities
                    menu_music.pause();
                    level_music.play(-1);
                    level_music.setVolume(50);
                    mousePressed = false; // Consume click
                } else if (onDesktop) {
                    gameRunning = false;
                }
            }
            // Render Menu
            SDL_SetRenderDrawColor(renderer, 50, 20, 10, 255); // Background
            SDL_RenderClear(renderer);
            SDL_RenderCopy(renderer, onStart ? startTextureHover : startTexture, NULL, &startButtonRect);
            SDL_RenderCopy(renderer, onDesktop ? desktopTextureHover : desktopTexture, NULL, &desktopButtonRect);

        } break;

        case GameState::PLAYING: {
            // Update Entities & Collisions
            entityManager.update(collisionTilemap, time, deltaTime); // Pass the collision map

            // Check for Game Over condition
            Player* player = entityManager.getPlayer();
            if (!player || !player->isAlive()) {
                currentState = GameState::GAME_OVER;
                level_music.stop();
                // Optionally play game over sound
                break; // Skip rendering this frame if game just ended
            }

            // Render Game World
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); // Black background
            SDL_RenderClear(renderer);
            
            // Draw all tilemaps in the correct order
            for (const auto& tilemap : tilemaps) {
                tilemap->draw(renderer, 0, 0);
            }
            
            entityManager.render();

            // Render HUD
            std::stringstream healthText;
            healthText << "Health: " << static_cast<int>(player->getHealth())
                       << " / " << static_cast<int>(player->getMaxHealth());
            SDL_Texture* healthTexture = renderText(renderer, hudFont, healthText.str(), white);
            if (healthTexture) {
                int healthW, healthH;
                SDL_QueryTexture(healthTexture, NULL, NULL, &healthW, &healthH);
                SDL_Rect healthRect = {10, 10, healthW, healthH};
                SDL_RenderCopy(renderer, healthTexture, NULL, &healthRect);
                SDL_DestroyTexture(healthTexture); // Clean up texture
            }

        } break;

        case GameState::PAUSED: {
             bool onContinue = SDL_PointInRect(&mousePoint, &continueButtonRect);
             bool onMenu = SDL_PointInRect(&mousePoint, &pauseMenuButtonRect);
             bool onDesktop = SDL_PointInRect(&mousePoint, &pauseDesktopButtonRect);

             if (mousePressed) {
                 if (onContinue) {
                     currentState = GameState::PLAYING;
                     level_music.setVolume(50); // Restore volume
                     mousePressed = false;
                 } else if (onMenu) {
                     currentState = GameState::MAIN_MENU;
                     entityManager.clearAll(); // Clear entities when going to menu
                     level_music.stop();
                     menu_music.play(-1);
                     mousePressed = false;
                 } else if (onDesktop) {
                     gameRunning = false;
                 }
             }

             // Render Paused State (Game world dimmed)
             SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
             SDL_RenderClear(renderer);
             
             // Draw all tilemaps in the correct order
             for (const auto& tilemap : tilemaps) {
                tilemap->draw(renderer, 0, 0);
             }
             
             entityManager.render(); // Render entities in their paused state

             // Dimming Overlay
             SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
             SDL_SetRenderDrawColor(renderer, 0, 0, 0, 128); // Semi-transparent black
             SDL_Rect overlayRect = {0, 0, 640, 480};
             SDL_RenderFillRect(renderer, &overlayRect);
             SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE); // Reset blend mode

             // Render Pause Menu Buttons
             SDL_RenderCopy(renderer, onContinue ? continueTextureHover : continueTexture, NULL, &continueButtonRect);
             SDL_RenderCopy(renderer, onMenu ? menuTextureHover : menuTexture, NULL, &pauseMenuButtonRect);
             SDL_RenderCopy(renderer, onDesktop ? desktopTextureHover : desktopTexture, NULL, &pauseDesktopButtonRect);

        } break;

        case GameState::GAME_OVER: {
             bool onRestart = SDL_PointInRect(&mousePoint, &restartButtonRect);
             bool onMenu = SDL_PointInRect(&mousePoint, &gameOverMenuButtonRect);
             bool onDesktop = SDL_PointInRect(&mousePoint, &gameOverDesktopButtonRect);

             if (mousePressed) {
                 if (onRestart) {
                     currentState = GameState::PLAYING;
                     setupNewGame(entityManager, renderer, handler); // Restart game
                     level_music.play(-1);
                     level_music.setVolume(50);
                     mousePressed = false;
                 } else if (onMenu) {
                     currentState = GameState::MAIN_MENU;
                     entityManager.clearAll();
                     menu_music.play(-1);
                     mousePressed = false;
                 } else if (onDesktop) {
                     gameRunning = false;
                 }
             }

             // Render Game Over State (Game world dimmed red)
             SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
             SDL_RenderClear(renderer);
             
             // Draw all tilemaps in the correct order
             for (const auto& tilemap : tilemaps) {
                tilemap->draw(renderer, 0, 0);
             }
             
             entityManager.render(); // Render entities (e.g., dead player)

             // Dimming Overlay (Red tint)
             SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
             SDL_SetRenderDrawColor(renderer, 100, 0, 0, 160); // Semi-transparent red
             SDL_Rect overlayRect = {0, 0, 640, 480};
             SDL_RenderFillRect(renderer, &overlayRect);
             SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);

             // Render Game Over Text & Buttons
             SDL_RenderCopy(renderer, gameOverTexture, NULL, &gameOverRect);
             SDL_RenderCopy(renderer, onRestart ? restartTextureHover : restartTexture, NULL, &restartButtonRect);
             SDL_RenderCopy(renderer, onMenu ? menuTextureHover : menuTexture, NULL, &gameOverMenuButtonRect);
             SDL_RenderCopy(renderer, onDesktop ? desktopTextureHover : desktopTexture, NULL, &gameOverDesktopButtonRect);

        } break;

        case GameState::QUIT: // Should not be reached in loop, but handle defensively
            gameRunning = false;
            break;
        }

        // --- Present Frame ---
        SDL_RenderPresent(renderer);

        // --- Cleanup Entities Marked for Deletion ---
        // Done once per frame, after updates and rendering potentially
        entityManager.cleanupEntities();

    } // End Main Game Loop

    // --- Cleanup ---
    entityManager.clearAll(); // Ensure all entities are cleared
    tilemaps.clear(); // Clear tilemaps

    // Destroy Textures
    SDL_DestroyTexture(startTexture);
    SDL_DestroyTexture(desktopTexture);
    SDL_DestroyTexture(startTextureHover);
    SDL_DestroyTexture(desktopTextureHover);
    SDL_DestroyTexture(continueTexture);
    SDL_DestroyTexture(menuTexture);
    SDL_DestroyTexture(continueTextureHover);
    SDL_DestroyTexture(menuTextureHover);
    SDL_DestroyTexture(gameOverTexture);
    SDL_DestroyTexture(restartTexture);
    SDL_DestroyTexture(restartTextureHover);

    // Close Fonts
    TTF_CloseFont(menuFont);
    TTF_CloseFont(hudFont);

    // Shutdown Systems
    AudioSystem::quit();
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_Quit();
    IMG_Quit();
    SDL_Quit();

    return 0;
}
