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

    keys.on_press["1"] = [&] { tool.set_current(&selector); };
    keys.on_press["2"] = [&] { tool.set_current(&painter); };
    keys.on_press["3"] = [&] { tool.set_current(&stroker); };

    keys.on_press["D"] = [&] { tool.dispatch(tags::debug); };

    auto paint = [&](auto p, int) { paint_layer.put(p, 1); };
    painter.set_method(paint);
    paint_layer.resize(uvec(320));
}

#define _observe Pz_observe_tag

void canvas::_init_observers()
{
    _observe(cursor.on_change) { tool.dispatch(tags::cursor_motion); };
    _observe(viewport.on_change) {
        viewport.update_uniforms(p_ui, false);
        tool.dispatch(tags::viewport);
    };
    // router.route(tags::viewport, controls.on_viewport_change);

    tool.add_shared_subjects(tool_hooks);
    tool.set_current(
        tool.add_tool(selector)
    );
    tool.add_tool(painter);
    tool.get_router(
        tool.add_tool(stroker)
    ).set_subject(stroker.on_edit);

    _observe(tool.on(tags::activate)) {
        viewport.on_change.dispatch({});
        cursor.on_change.dispatch({});
    };
    _observe(tool.on(tags::viewport)) {
        viewport.update_uniforms(p_quad);
        set(p_lines->mvp_matrix, viewport.view_matrix());
    };

    _observe(selector.on(tags::cursor_motion)) { selector.update_cursor(b_ui); };
    _observe(selector.on(tags::deactivate)) { b_ui.clear(); b_ui.update(); };
    _observe(selector.on_selection) {
        b_ui.clear(); b_ui.update();
        selector.update_selection(b_model);
    };

    _observe(painter.on_edit) {
        b_paint.clear();
        paint_layer.for_each([&] (auto pos, auto& cell) {
            if (!cell) return;
            b_paint.push(pos, uvec(1), rgba(Rxt::colors::sand, 1));
        });
        b_paint.update();
    };

    _observe(stroker.on(tags::cursor_motion)) {
        stroker.update_cursor(b_lines_cursor);
    };
    _observe(stroker.on_edit) {
        stroker.update_cursor(b_lines_cursor);
        stroker.update_model(b_lines);
    };
    _observe(stroker.on(tags::debug)) {
        if (stroker._current)
            Rxt_DEBUG(stroker._current->at(0));
        else print("nothing\n");
    };

    set(p_ui->viewport_position, ivec{0});
    viewport.on_change.dispatch({}); // set initial viewport

    router.set_subject(cursor.on_change);
    router.set_subject(viewport.on_change);
    router.set_subject(selector.on_selection);
    router.set_subject(painter.on_edit);
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
    auto dirty = router.flush();
    dirty += tool.flush();

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

void canvas::handle_mouse_down(SDL_MouseButtonEvent button)
{
    tool.mouse_down(mouse_button_from_sdl(button));
}

void canvas::handle_mouse_up(SDL_MouseButtonEvent button)
{
    tool.mouse_up(mouse_button_from_sdl(button));
}
