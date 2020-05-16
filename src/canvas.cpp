#include "canvas.hpp"
#include <Rxt/range.hpp>
#include <iostream>

namespace gl = Rxt::gl;
namespace sdl = Rxt::sdl;
using Rxt::print;

extern "C" void step_state(void* c)
{
    sdl::step<canvas>(c);
}

int main(int argc, char** argv)
{
    int seed = 42;
    if (argc > 1) {
        seed = std::stoi(argv[1]);
    }

    try {
        auto loop = sdl::make_looper(
            new canvas(grid_viewport{uvec(640)}),
            step_state
        );
        loop();
    } catch (std::exception& e) {
        std::cout << "caught exception: " << e.what() << '\n';
    }

    return 0;
}

canvas::canvas(grid_viewport vp)
    : simple_gui("plaza: canvas", vp.size_pixels())
    , viewport{vp}
    , tool(&selector)
{
    keys.on_press["C-W"] = [this] { quit = true; };

    set(p_ui.u_.viewport_position, ivec{0});
    set(p_ui.u_.viewport_size, vp.size_cells());

    Pz_observe(viewport, auto& vp) {
        set(p_obj.u_.viewport_position, vp.position);
        set(p_obj.u_.viewport_size, vp.size_cells());
        std::cout << "viewport=(@:" << vp.position << ", s:" << vp.size_cells() << ")\n";
        set_dirty();
    };

    Pz_observe(cursor, auto& c) {
        cursor.update(b_ui);
        std::cout << "cursor=" << cursor.position << "\n";
        set_dirty();
    };

    Pz_observe(selector, auto& s) {
        selector.update(b_ui, b_obj);
        set_dirty();
    };

    notify_observers(viewport);

    glClearColor(0, 0, 0, 1);
}

void canvas::step(SDL_Event event)
{
    do {
        handle_input(event);
    } while (SDL_PollEvent(&event));

    keys.scan();

    // Per-tick handlers
    if (enable_edge_scroll) {
        viewport.edge_scroll(cursor.position, 1);
    }

    if (is_dirty()) {
        draw();
        set_dirty(false);
    }
}

void canvas::draw()
{
    glClear(GL_COLOR_BUFFER_BIT);

    b_ui.draw();
    b_obj.draw();

    SDL_GL_SwapWindow(window.get());
}

void canvas::handle_mouse_down(SDL_MouseButtonEvent button)
{
    switch (button.button) {
    case SDL_BUTTON_LEFT: {
        if (tool) tool->mouse_down(0);
        break;
    }
    case SDL_BUTTON_RIGHT: {
        if (tool) tool->mouse_down(1);
        break;
    }
    }
}

void canvas::handle_mouse_up(SDL_MouseButtonEvent button)
{
    switch (button.button) {
    case SDL_BUTTON_LEFT: {
        if (tool) tool->mouse_up(0);
        break;
    }
    }
}
