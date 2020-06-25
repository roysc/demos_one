#include "canvas.hpp"
#include "util.hpp"

#include <Rxt/graphics/gl_handy.hpp>
#include <Rxt/range.hpp>
#include <Rxt/log.hpp>
#include <iostream>

namespace gl = Rxt::gl;
namespace sdl = Rxt::sdl;
using Rxt::print;

extern "C" void step_state(void* c)
{
    sdl::em_advance<canvas>(c);
}

int main(int argc, char** argv)
{
    int seed = 42;
    if (argc > 1) {
        seed = std::stoi(argv[1]);
    }

    gl::debug_context::enable_logging = false;

    auto loop = sdl::make_looper(
        new canvas(viewport_type{uvec(80), uvec(8)}),
        step_state
    );
    loop();

    return 0;
}

void canvas::_init_observers()
{
    router.insert(&selector);
    router.insert(&stroker);
    router.insert(&painter);

    PZ_observe(cursor.on_update) {
        router->on_cursor_update();
    };
    PZ_observe(viewport.on_update) {
        viewport.update_uniforms(p_ui, false);
        router->on_viewport_update();
    };
    PZ_observe(router->on_viewport_update) {
        viewport.update_uniforms(p_quad);
        set(p_lines->mvp_matrix, viewport.view_matrix());
    };

    PZ_observe(router->on_enable) {
        viewport.on_update();
        cursor.on_update();
    };

    PZ_observe(selector.on_selection) {
        b_ui.clear(); b_ui.update();
        selector.update_selection(b_model);
    };
    PZ_observe(selector.on_motion) {
        cursor.on_update();
    };
    PZ_observe(router[&selector].on_cursor_update) {
        selector.update_cursor(b_ui);
    };
    PZ_observe(router[&selector].on_disable) {
        b_ui.clear(); b_ui.update();
    };

    PZ_observe(painter.on_edit) {
        b_paint.clear();
        paint_layer.for_each([&] (auto pos, auto& cell) {
            if (!cell) return;
            b_paint.push(pos, uvec(1), rgba(Rxt::colors::sand, 1));
        });
        b_paint.update();
    };

    PZ_observe(stroker.on_edit) {
        stroker.update_cursor(b_lines_cursor);
        stroker.update_model(b_lines);
    };
    PZ_observe(router[&stroker].on_cursor_update) {
        stroker.update_cursor(b_lines_cursor);
    };
    PZ_observe(router[&stroker].on_debug) {
        if (stroker._current)
            RXT_show(stroker._current->at(0));
        else print("nothing\n");
    };

    PZ_observe(router->on_debug) {
        print("cursor={}\nviewport=(pos={}, scale={})\n",
              cursor.position(), viewport.position(), viewport.scale_factor
        );
    };

    set(p_ui->viewport_position, ivec{0});
    viewport.on_update(); // set initial viewport
}

void canvas::_init_controls()
{
    keys.on_press["C-W"] = [this] { quit = true; };

    auto move = [this] (int dx, int dy) { viewport.move(ivec(dx, dy)); };
    auto scale_center = [this] (int a) { viewport.set_scale(viewport.scale_pow(a)); };

    keys.on_scan["Left"]  = std::bind(move, -1, 0);
    keys.on_scan["Right"] = std::bind(move, +1, 0);
    keys.on_scan["Down"]  = std::bind(move, 0, -1);
    keys.on_scan["Up"]    = std::bind(move, 0, +1);
    keys.on_press["."] = std::bind(scale_center, +1);
    keys.on_press[","] = std::bind(scale_center, -1);

    auto set_tool = [this] (auto t) { tool = t; router.enable(t); };
    keys.on_press["1"] = std::bind(set_tool, &selector);
    keys.on_press["2"] = std::bind(set_tool, &painter);
    keys.on_press["3"] = std::bind(set_tool, &stroker);
    set_tool(&selector);

    keys.on_press["D"] = [&] { router->on_debug(); };

    auto paint = [this](auto p, int) { paint_layer.put(p, 1); };
    painter.set_method(paint);
    paint_layer.resize(uvec(320));

    PZ_observe(input.on_mouse_motion, SDL_MouseMotionEvent motion) {
        auto [x, y] = Rxt::sdl::nds_coords(*window, motion.x, motion.y);
        auto gridpos = viewport.from_nds(x, y);
        cursor.position(gridpos);
    };
    PZ_observe(input.on_quit) { quit = true; };
    PZ_observe(input.on_key_down, SDL_Keysym k) { keys.press(k); };
    PZ_observe(input.on_mouse_wheel, SDL_MouseWheelEvent wheel) {
        if (wheel.y == 0) return;
        auto sf = viewport.scale_pow(-wheel.y);
        viewport.scale_to(sf, controls.cursor_worldspace());
    };

    PZ_observe(input.on_mouse_down, SDL_MouseButtonEvent button) {
        tool->mouse_down(mouse_button_from_sdl(button));
    };
    PZ_observe(input.on_mouse_up, SDL_MouseButtonEvent button) {
        tool->mouse_up(mouse_button_from_sdl(button));
    };
}

void canvas::advance(SDL_Event event)
{
    do {
        input.handle_input(event);
    } while (SDL_PollEvent(&event));

    keys.scan();

    // Per-tick handlers
    if (enable_edge_scroll) {
        viewport.edge_scroll(cursor.position(), 1);
    }

    auto updates = {
        &cursor.on_update,
        &viewport.on_update,
        &router.on_update,
    };
    auto dirty = Rxt::flush_all(updates);

    if (dirty) draw();
}

void canvas::draw()
{
    glClear(GL_COLOR_BUFFER_BIT);

    b_model.draw();
    b_paint.draw();
    b_lines.draw();

    b_ui.draw();
    b_lines_cursor.draw();

    SDL_GL_SwapWindow(window.get());
}
