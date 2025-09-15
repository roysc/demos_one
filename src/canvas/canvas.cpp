#include "canvas.hpp"

#include <Rxt/graphics/gl/handy.hpp>
#include <Rxt/log.hpp>
#include <Rxt/range.hpp>


namespace gl = Rxt::gl;
namespace sdl = Rxt::sdl;
using Rxt::print;
using Rxt::vec::uvec2;

mouse_button mouse_button_from_sdl(SDL_MouseButtonEvent button);

int main(int, char**)
{
    gl::get_config().enable_logging = false;

    sdl::run_loop(new canvas(viewport_type{uvec2(80), uvec2(8)}));
    return 0;
}

void canvas::_init_observers()
{
    router.insert(&selector);
    router.insert(&stroker);
    router.insert(&painter);

    cursor.on_update += [this] {
        router->on_cursor_update();
    };
    // The UI heeds updates in viewport size, but not position
    viewport.on_update += [this] {
        set(p_ui->viewport_size, viewport.size_cells());
        router->on_viewport_update();
    };
    // Model programs heed all viewport updates
    router->on_viewport_update += [this] {
        set(p_quad->viewport_size, viewport.size_cells());
        set(p_quad->viewport_position, viewport.position());
        set(p_lines->mvp_matrix, viewport.view_matrix());
    };

    router->on_enable += [this] {
        viewport.on_update();
        cursor.on_update();
    };

    selector.on_selection += [this] {
        b_ui.clear();
        b_ui.update();
        selector.render_selection(b_model);
    };
    selector.on_motion += [this] {
        cursor.on_update();
    };
    router[&selector].on_cursor_update += [this] {
        selector.render_cursor(b_ui);
    };
    router[&selector].on_disable += [this] {
        b_ui.clear();
        b_ui.update();
    };

    painter.on_edit += [this] {
        b_paint.clear();
        paint_layer.for_each([&](auto pos, auto& cell) {
            if (!cell)
                return;
            b_paint.push(ivec3(pos, 0), uvec2(1), rgba(Rxt::colors::sand, 1));
        });
        b_paint.update();
    };

    stroker.on_edit += [this] {
        stroker.render_cursor(b_lines_cursor);
        stroker.render_model(b_lines);
    };
    router[&stroker].on_cursor_update += [this] {
        stroker.render_cursor(b_lines_cursor);
    };
    router[&stroker].on_debug += [this] {
        if (stroker._current)
            stroker._current->at(0);
        else
            print("nothing\n");
    };

    router->on_debug += [this] {
        print("cursor={}\nviewport=(pos={}, scale={})\n", cursor.position(), viewport.position(),
              viewport.scale_factor());
    };

    set(p_ui->viewport_position, ivec2{0});
    viewport.on_update(); // set initial viewport
}

void canvas::_init_controls()
{
    auto move = [this](int dx, int dy) { viewport.translate(ivec2(dx, dy)); };
    auto scale_center = [this](int a) { viewport.set_scale(viewport.scale_pow(a)); };
    auto set_tool = [this](auto t) {
        tool = t;
        router.enable(t);
    };
    auto paint = [this](auto p, int) { paint_layer.put(p, 1); };

    set_tool(&selector);
    painter.set_method(paint);
    paint_layer.resize(uvec2(320));

    keys.on_press["C-W"] = [this] { quit = true; };
    keys.on_scan["Left"] = std::bind(move, -1, 0);
    keys.on_scan["Right"] = std::bind(move, +1, 0);
    keys.on_scan["Down"] = std::bind(move, 0, -1);
    keys.on_scan["Up"] = std::bind(move, 0, +1);
    keys.on_press["."] = std::bind(scale_center, +1);
    keys.on_press[","] = std::bind(scale_center, -1);
    keys.on_press["1"] = std::bind(set_tool, &selector);
    keys.on_press["2"] = std::bind(set_tool, &painter);
    keys.on_press["3"] = std::bind(set_tool, &stroker);
    keys.on_press["D"] = [&] { router->on_debug(); };

    input.on_mouse_motion += [this] (SDL_MouseMotionEvent motion) {
        auto [x, y] = Rxt::sdl::nds_coords(window(), motion.x, motion.y);
        auto gridpos = viewport.from_nds(x, y);
        cursor.set_position(gridpos);
    };
    input.on_quit += [this] {
        quit = true;
    };
    input.on_key_down += [this] (SDL_Keysym k) {
        keys.press(k);
    };
    input.on_mouse_wheel += [this] (SDL_MouseWheelEvent wheel) {
        if (wheel.y == 0)
            return;
        auto sf = viewport.scale_pow(-wheel.y);
        viewport.set_scale_focused(sf, controls.cursor_worldspace());
    };
    input.on_mouse_down += [this] (SDL_MouseButtonEvent button) {
        tool->mouse_down(mouse_button_from_sdl(button));
    };
    input.on_mouse_up += [this] (SDL_MouseButtonEvent button) {
        tool->mouse_up(mouse_button_from_sdl(button));
    };
}

void canvas::advance(Rxt::loop_duration)
{
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        input.handle_input(event);
    }
    keys.scan();

    // Per-tick handlers
    if (enable_edge_scroll) {
        viewport.edge_scroll(cursor.position(), 1);
    }

    draw();
}

void canvas::draw()
{
    glClear(GL_COLOR_BUFFER_BIT);

    b_model.draw();
    b_paint.draw();
    b_lines.draw();

    b_ui.draw();
    b_lines_cursor.draw();

    SDL_GL_SwapWindow(window());
}

mouse_button mouse_button_from_sdl(SDL_MouseButtonEvent button)
{
    switch (button.button) {
    case SDL_BUTTON_LEFT:
        return mouse_button::left;
    case SDL_BUTTON_MIDDLE:
        return mouse_button::middle;
    case SDL_BUTTON_RIGHT:
        return mouse_button::right;
    default:
        return mouse_button::invalid;
    }
}
