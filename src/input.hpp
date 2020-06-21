#pragma once
#include "reactive.hpp"
#include <Rxt/graphics/sdl.hpp>

struct input_hooks : Rxt::sdl::input_handler<input_hooks>
{
    Rxt::hooks<> on_quit;
    Rxt::hooks<SDL_Keysym> on_key_down;
    Rxt::hooks<SDL_MouseButtonEvent> on_mouse_down, on_mouse_up;
    Rxt::hooks<SDL_MouseMotionEvent> on_mouse_motion;
    Rxt::hooks<SDL_MouseWheelEvent> on_mouse_wheel;
};
