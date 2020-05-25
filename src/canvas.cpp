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

    auto paint = [&](auto p, int) { paint_layer[p.x][p.y] = 1; };
    painter.set_method(paint);
    auto size = uvec(320);
    paint_layer.resize(boost::extents[size.x][size.y]);
}

canvas::canvas(grid_viewport vp)
    : simple_gui("plaza: canvas", vp.size_pixels())
    , controls{vp}
{
    _set_controls();

    router.add_subject(controls.on_viewport_change);
    router.add_subject(controls.on_motion);
    router.add_subject(selector.on_selection);
    router.add_subject(painter.on_edit);

    // auto selector_on = tool.add_tool(&selector, &tool_hooks.at(1));
    tool.add_shared_subjects(tool_hooks.at(0));
    auto selector_on = tool.add_tool(&selector, &tool_hooks.at(1));
    auto painter_on = tool.add_tool(&painter, &tool_hooks.at(2));
    auto stroker_on = tool.add_tool(&stroker, &tool_hooks.at(3));

    Pz_observe(controls.on_motion) {
        tool.dispatch(tags::cursor_motion);
    };
    Pz_observe(controls.on_viewport_change) {
        viewport.update_uniforms(p_ui, false);
        tool.dispatch(tags::viewport);
    };

    Pz_observe(selector.on_selection) {
        selector.update_selection(b_model);
        if (selector.selection) {
            auto [a, b] = *selector.selection;
            std::cout << "selection=(" << a << ", " << b << ")\n";
        } else
            std::cout << "selection=null\n";
    };

    Pz_observe(tool.get_shared(tags::activate)) {
        controls.on_viewport_change.notify({});
        controls.on_motion.notify({});
    };

    // tool.add_subject(tags::viewport, i, tool_hooks[1].get(tags::viewport));
    Pz_observe(selector_on(tags::viewport)) { viewport.update_uniforms(p_model); };
    Pz_observe(selector_on(tags::cursor_motion)) { selector.update_cursor(b_ui); };
    Pz_observe(selector_on(tags::deactivate)) { b_ui.clear(); b_ui.update(); };

    Pz_observe(painter.on_edit) {
        b_paint.clear();
        auto shape = paint_layer.shape();
        for (unsigned y = 0; y < shape[1]; ++y) {
            for (unsigned x = 0; x < shape[0]; ++x) {
                auto cell = paint_layer[x][y];
                if (cell != 0) {
                    b_paint.push(ivec(x, y), uvec(1), rgba(Rxt::colors::sand, 1));
                }
            }
        }
        b_paint.update();
    };

    Pz_observe(stroker.on_edit) {
        b_lines_cursor.clear();
        stroker.update_model(b_lines);
    };

    Pz_observe(stroker_on(tags::viewport)) {
        // auto mvp_matrix = viewport.view_matrix() * viewport.model_matrix();
        auto mvp_matrix = viewport.view_matrix();
        set(p_lines->mvp_matrix, mvp_matrix);
    };
    Pz_observe(stroker_on(tags::cursor_motion)) {
        b_lines_cursor.clear();
        stroker.update_cursor(b_lines_cursor);
    };
    Pz_observe(stroker_on(tags::debug)) {
        if (stroker._current)
            Rxt_DEBUG(stroker._current->at(0));
        else Rxt::print("nothing\n");
    };

    set(p_ui->viewport_position, ivec{0});

    controls.on_viewport_change.notify({});
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
    for (auto& hook: tool_hooks) dirty += hook.flush();

    if (dirty) draw();
}

void canvas::draw()
{
    glClear(GL_COLOR_BUFFER_BIT);

    // draw_if_dirty(b_model);

    b_paint.draw();
    b_model.draw();
    b_ui.draw();
    b_lines.draw();
    b_lines_cursor.draw();

    SDL_GL_SwapWindow(window.get());
}

void canvas::handle_mouse_down(SDL_MouseButtonEvent button)
{
    switch (button.button) {
    case SDL_BUTTON_LEFT: {
        tool.mouse_down(0);
        break;
    }
    case SDL_BUTTON_RIGHT: {
        tool.mouse_down(1);
        break;
    }
    }
}

void canvas::handle_mouse_up(SDL_MouseButtonEvent button)
{
    switch (button.button) {
    case SDL_BUTTON_LEFT: {
        tool.mouse_up(0);
        break;
    }
    }
}
