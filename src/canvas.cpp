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

    // try {
    auto loop = sdl::make_looper(
        new canvas(grid_viewport{uvec(80), uvec(8)}),
        step_state
    );
    loop();
    // } catch (std::exception& e) { Rxt::print("caught: {}\n", e.what()); }

    return 0;
}

void canvas::_set_controls()
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
    // painter.set_method(std::bind(&dense_map<int>::put, paint_layer));
    paint_layer.resize(uvec(320));
}

canvas::canvas(grid_viewport vp)
    : simple_gui("plaza: canvas", vp.size_pixels())
    , controls{vp}
{
    _set_controls();

    router.set_subject(controls.on_viewport_change);
    router.set_subject(controls.on_motion);
    router.set_subject(selector.on_selection);
    router.set_subject(painter.on_edit);

    tool.add_shared_subjects(tool_hooks);

    tool.set_current(
        tool.add_tool(selector)
    );
    tool.add_tool(painter);
    tool.get_router(
        tool.add_tool(stroker)
    ).set_subject(stroker.on_edit);

    Pz_observe(tool.on(tags::activate)) {
        controls.on_viewport_change.dispatch({});
        controls.on_motion.dispatch({});
    };
    Pz_observe(tool.on(tags::viewport)) {
        viewport.update_uniforms(p_quad);
        set(p_lines->mvp_matrix, viewport.view_matrix());
    };

    Pz_observe(controls.on_motion) { tool.dispatch(tags::cursor_motion); };
    Pz_observe(controls.on_viewport_change) {
        viewport.update_uniforms(p_ui, false);
        tool.dispatch(tags::viewport);
    };
    //
    Pz_observe(selector.on(tags::cursor_motion)) { selector.update_cursor(b_ui); };
    Pz_observe(selector.on(tags::deactivate)) { b_ui.clear(); b_ui.update(); };
    Pz_observe(selector.on_selection) {
        b_ui.clear(); b_ui.update();
        selector.update_selection(b_model);
    };
    //
    Pz_observe(painter.on_edit) {
        b_paint.clear();
        paint_layer.for_each([&] (auto pos, auto& cell) {
            if (!cell) return;
            b_paint.push(pos, uvec(1), rgba(Rxt::colors::sand, 1));
        });
        b_paint.update();
    };
    //
    Pz_observe(stroker.on(tags::cursor_motion)) {
        stroker.update_cursor(b_lines_cursor);
    };
    Pz_observe(stroker.on_edit) {
        stroker.update_cursor(b_lines_cursor);
        stroker.update_model(b_lines);
    };
    Pz_observe(stroker.on(tags::debug)) {
        if (stroker._current)
            Rxt_DEBUG(stroker._current->at(0));
        else Rxt::print("nothing\n");
    };

    set(p_ui->viewport_position, ivec{0});

    controls.on_viewport_change.dispatch({});
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
        viewport.edge_scroll(controls.cursor_position(), 1);
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
