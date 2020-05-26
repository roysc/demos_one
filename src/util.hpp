#pragma once
#include "mouse_core.hpp"

#include <Rxt/io.hpp>
#include <SDL2/SDL.h>

template <unsigned ix, class Vec>
Vec invert(Vec v) { v[ix] = -v[ix]; return v; }

inline mouse_button mouse_button_from_sdl(SDL_MouseButtonEvent button)
{
    switch (button.button) {
    case SDL_BUTTON_LEFT: return mouse_button::left;
    case SDL_BUTTON_MIDDLE: return mouse_button::middle;
    case SDL_BUTTON_RIGHT: return mouse_button::right;
    default: return mouse_button::invalid;
    }
}
