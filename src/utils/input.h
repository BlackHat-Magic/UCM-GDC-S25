#pragma once
#include <SDL2/SDL.h>

class InputHandler
{
public:
    InputHandler() = default;
    ~InputHandler() = default;

    void handle_keydown(SDL_Keycode key);
    void handle_keyup(SDL_Keycode key);
    void handle_mousemotion(int x, int y);
    void handle_mousebuttondown(Uint8 button, int x, int y);
    void handle_mousebuttonup(Uint8 button, int x, int y);

    bool is_key_pressed(SDL_Scancode key) const;
    bool is_mouse_button_pressed(Uint8 button) const;
    std::pair<int, int> get_mouse_position() const;

private:
    int mouse_x = 0, mouse_y = 0;
    Uint8 key_states[SDL_NUM_SCANCODES] = {0};
    Uint8 mouse_button_states[5] = {0};
};