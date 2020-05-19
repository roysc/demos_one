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

    keys.on_press["C"] = [&] { mouse_tool = &selector; set_dirty(); };

    // keys.on_press["Space"] = [&] { _lines.add(selector.cursor_position()); };
    // keys.on_press["W"] =[&] { _lines.close(); };
 }

canvas::canvas(grid_viewport vp)
    : simple_gui("plaza: canvas", vp.size_pixels())
    , viewport{vp}
{
    _set_controls();

    viewport.set_router(obr);
    selector.set_router(obr);

    set(p_ui.u_.viewport_position, ivec{0});

    // Pz_observe(obs, viewport)
    // Pz_observe(obs, cursor_motion)
    // Pz_observe(obs, cursor_selection)

    // actions.add
    Pz_observe(obr, tag::viewport) {
        set(p_ui.u_.viewport_size, viewport.size_cells());
        set(p_model.u_.viewport_position, viewport.position());
        set(p_model.u_.viewport_size, viewport.size_cells());
        set_dirty();
    };
    // Pz_notify(viewport);

    Pz_observe(obr, tag::cursor_motion) {
        selector.update_cursor(b_ui);
        set_dirty();
    };

    Pz_observe(obr, tag::cursor_selection) {
        selector.update_selection(b_model);
        if (selector.selection) {
            auto [a, b] = *selector.selection;
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
        viewport.edge_scroll(selector.cursor_position(), 1);
    }

    // Pz_flush(viewport);
    // Pz_flush_on(selector, motion);
    // Pz_flush_on(selector, selection);

    if (is_dirty()) {
        draw();
        set_dirty(false);
    }
}

void canvas::draw()
{
    glClear(GL_COLOR_BUFFER_BIT);

    b_model.draw();
    b_ui.draw();

    SDL_GL_SwapWindow(window.get());
}

void canvas::handle_mouse_down(SDL_MouseButtonEvent button)
{
    switch (button.button) {
    case SDL_BUTTON_LEFT: {
        if (mouse_tool) mouse_tool->mouse_down(0);
        break;
    }
    case SDL_BUTTON_RIGHT: {
        if (mouse_tool) mouse_tool->mouse_down(1);
        break;
    }
    }
}

void canvas::handle_mouse_up(SDL_MouseButtonEvent button)
{
    switch (button.button) {
    case SDL_BUTTON_LEFT: {
        if (mouse_tool) mouse_tool->mouse_up(0);
        break;
    }
    }
}
