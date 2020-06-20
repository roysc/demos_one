#include "canvas.hpp"
#include "util.hpp"

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

    auto loop = sdl::make_looper(
        new canvas(viewport_type{uvec(80), uvec(8)}),
        step_state
    );
    loop();

    return 0;
}

#define PZ_observe Pz_observe

void canvas::_init_observers()
{
    th.insert(&selector);
    th.insert(&stroker);
    th.insert(&painter);

    PZ_observe(cursor.on_update) {
        th.on_cursor_update();
    };
    PZ_observe(viewport.on_update) {
        viewport.update_uniforms(p_ui, false);
        th.on_viewport_update();
    };
    PZ_observe(th.on_viewport_update) {
        viewport.update_uniforms(p_quad);
        set(p_lines->mvp_matrix, viewport.view_matrix());
    };

    PZ_observe(th.on_enable) {
        viewport.on_update();
        cursor.on_update();
    };

    PZ_observe(th[&selector].on_cursor_update) {
        selector.update_cursor(b_ui);
    };
    PZ_observe(th[&selector].on_disable) {
        b_ui.clear(); b_ui.update();
    };
    // PZ_observe(th[&selector].on_select) {
    PZ_observe(selector.on_selection) {
        b_ui.clear(); b_ui.update();
        selector.update_selection(b_model);
    };

    // PZ_observe(th[&painter].on_edit) {
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
    PZ_observe(th[&stroker].on_cursor_update) {
        stroker.update_cursor(b_lines_cursor);
    };
    PZ_observe(th[&stroker].on_debug) {
        if (stroker._current)
            Rxt_DEBUG(stroker._current->at(0));
        else print("nothing\n");
    };

    set(p_ui->viewport_position, ivec{0});
    viewport.on_update(); // set initial viewport

    // router.set_subject(cursor.on_update);
    // router.set_subject(viewport.on_update);
    // router.set_subject(selector.on_selection);
    // router.set_subject(painter.on_edit);
}

void canvas::_init_controls()
{
    keys.on_press["C-W"] = [this] { quit = true; };

    auto move = [this] (int dx, int dy) { viewport.move(ivec(dx, dy)); };
    auto scale = [this] (int a) { viewport.scale(a); };

    keys.on_scan["Left"]  = std::bind(move, -1, 0);
    keys.on_scan["Right"] = std::bind(move, +1, 0);
    keys.on_scan["Down"]  = std::bind(move, 0, -1);
    keys.on_scan["Up"]    = std::bind(move, 0, +1);
    keys.on_press["."] = std::bind(scale, +1);
    keys.on_press[","] = std::bind(scale, -1);

    auto set_tool = [this] (auto t) { tool = t; th.enable(t); };
    keys.on_press["1"] = std::bind(set_tool, &selector);
    keys.on_press["2"] = std::bind(set_tool, &painter);
    keys.on_press["3"] = std::bind(set_tool, &stroker);
    set_tool(&selector);

    keys.on_press["D"] = [&] { th.on_debug(); };

    auto paint = [this](auto p, int) { paint_layer.put(p, 1); };
    painter.set_method(paint);
    paint_layer.resize(uvec(320));
}

void canvas::step(SDL_Event event)
{
    do {
        handle_input(event);
    } while (SDL_PollEvent(&event));

    keys.scan();

    // Per-tick handlers
    if (enable_edge_scroll) {
        viewport.edge_scroll(cursor.position(), 1);
    }

    // Ideally we can track everything from flush() calls
    // auto dirty = router.flush();
    // dirty += tool.flush();

    auto updates = {
        &cursor.on_update,
        &viewport.on_update,
        // &th.on_update
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

void canvas::on_mouse_down(SDL_MouseButtonEvent button)
{
    tool->mouse_down(mouse_button_from_sdl(button));
}

void canvas::on_mouse_up(SDL_MouseButtonEvent button)
{
    tool->mouse_up(mouse_button_from_sdl(button));
}
