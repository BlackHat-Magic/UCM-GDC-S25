#include "input.h"
#include <SDL2/SDL.h>

void InputHandler::handle_keydown(SDL_Keycode key) {
    SDL_Scancode scancode = SDL_GetScancodeFromKey(key);
    if (scancode < SDL_NUM_SCANCODES) {
        key_states[scancode] = 1;
    }
}

void InputHandler::handle_keyup(SDL_Keycode key) {
    SDL_Scancode scancode = SDL_GetScancodeFromKey(key);
    if (scancode < SDL_NUM_SCANCODES) {
        key_states[scancode] = 0;
    }
}

void InputHandler::handle_mousemotion(int x, int y) {
    mouse_x = x;
    mouse_y = y;
}

void InputHandler::handle_mousebuttondown(Uint8 button, int x, int y) {
    if (button <= 4) {
        mouse_button_states[button] = 1;
    }
    mouse_x = x;
    mouse_y = y;
}

void InputHandler::handle_mousebuttonup(Uint8 button, int x, int y) {
    if (button <= 4) {
        mouse_button_states[button] = 0;
    }
    mouse_x = x;
    mouse_y = y;
}

bool InputHandler::is_key_pressed(SDL_Scancode key) const {
    if (key < SDL_NUM_SCANCODES) {
        return key_states[key] == 1;
    }
    return false;
}

bool InputHandler::is_mouse_button_pressed(Uint8 button) const {
    if (button <= 4) {
        return mouse_button_states[button] == 1;
    }
    return false;
}

std::pair<int, int> InputHandler::get_mouse_position() const {
    return std::make_pair(mouse_x, mouse_y);
}
