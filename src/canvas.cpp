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
            new canvas(grid_viewport{uvec(80), uvec(8)}),
            step_state
        );
        loop();
    } catch (std::exception& e) {
        std::cout << "caught exception: " << e.what() << '\n';
    }

    return 0;
}

void canvas::_set_controls()
{
    keys.on_press["C-W"] = [this] { quit = true; };

    auto move = [this] (int dx, int dy) { viewport.move(dx, dy); set_dirty(); };
    auto scale = [this] (int a) { viewport.scale(a); set_dirty(); };

    keys.on_scan["Left"]  = std::bind(move, -1, 0);
    keys.on_scan["Right"] = std::bind(move, +1, 0);
    keys.on_scan["Down"]  = std::bind(move, 0, -1);
    keys.on_scan["Up"]    = std::bind(move, 0, +1);
    keys.on_press["."] = std::bind(scale, +1);
    keys.on_press[","] = std::bind(scale, -1);

    keys.on_press["C"] = [&] { cursor_tool = &cursor; set_dirty(); };
    keys.on_press["S"] = [&] { cursor_tool = &selector; set_dirty(); };
}

canvas::canvas(grid_viewport vp)
    : simple_gui("plaza: canvas", vp.size_pixels())
    , viewport{vp}
    , cursor_tool(&selector)
{
    _set_controls();

    set(p_ui.u_.viewport_position, ivec{0});

    Pz_observe(viewport, auto& vp) {
        set(p_ui.u_.viewport_size, viewport.size_cells());
        set(p_obj.u_.viewport_position, vp.position);
        set(p_obj.u_.viewport_size, vp.size_cells());
        set_dirty();
    };
    notify_observers(viewport);

    Pz_observe_on(cursor, cursor, auto& c) {
        c.update_cursor(b_ui);
        set_dirty();
    };

    Pz_observe_on(selector, cursor, auto& s) {
        s.update_cursor(b_ui);
        set_dirty();
    };

    Pz_observe_on(selector, selection, auto& s) {
        s.update_selection(b_obj);
        if (s.selection) {
            auto [a, b] = *s.selection;
            std::cout << "selection=(" << a << ", " << b << ")\n";
        } else
            std::cout << "selection=null\n";
        set_dirty();
    };

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
        viewport.edge_scroll(cursor_tool->get_position(), 1);
    }

    if (is_dirty()) {
        draw();
        set_dirty(false);
    }
}

void canvas::draw()
{
    glClear(GL_COLOR_BUFFER_BIT);

    b_obj.draw();
    b_ui.draw();

    SDL_GL_SwapWindow(window.get());
}

void canvas::handle_mouse_down(SDL_MouseButtonEvent button)
{
    switch (button.button) {
    case SDL_BUTTON_LEFT: {
        if (cursor_tool) cursor_tool->mouse_down(0);
        break;
    }
    case SDL_BUTTON_RIGHT: {
        if (cursor_tool) cursor_tool->mouse_down(1);
        break;
    }
    }
}

void canvas::handle_mouse_up(SDL_MouseButtonEvent button)
{
    switch (button.button) {
    case SDL_BUTTON_LEFT: {
        if (cursor_tool) cursor_tool->mouse_up(0);
        break;
    }
    }
}
