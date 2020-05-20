#pragma once

#include "mouse.hpp"

#include <Rxt/graphics/sdl.hpp>
#include <Rxt/graphics/shader/grid_quad_2D.hpp>

struct grid_traits
{
    using position_type = ivec;
    using size_type = uvec;
};

using grid_program = Rxt::shader_programs::webcompat::grid_quad_2D;
using grid_viewport = viewport<grid_traits>;

using ui_traits = mouse_ui<grid_traits>;
using grid_selector = mouse_select_tool<grid_traits>;
using grid_painter = mouse_paint_tool<grid_traits>;

using obrouter = observer_router<
    // eager_observable,
    // lazy_observable,
    tags::viewport, tags::cursor_motion, tags::cursor_selection
    >;

struct model_buffers : grid_program::data
{
    const Rxt::rgba select_color {0, 0, 1, 1};

    void add_selection(ivec a, ivec b)
    {
        clear();
        push(a, b-a+1, select_color);
        update();
    }
};

struct ui_buffers : grid_program::data
{
    void set_cursor(position_vec p, size_vec s, Rxt::rgba color)
    {
        clear();
        push(p, s, color);
        update();
    }
};

struct canvas
    : Rxt::sdl::simple_gui
    , Rxt::sdl::input_handler<canvas>
{
    Rxt::sdl::key_dispatcher keys;
    bool enable_edge_scroll = true;
    bool quit = false;

    std::vector<ivec> _line_points;

    obrouter obr;

    grid_viewport viewport;
    grid_selector selector {viewport};
    grid_painter tile_painter {selector, [&](ivec p, int){_line_points.emplace_back(p);}};
    mouse_tool* mouse_tool {&selector};

    grid_program p_ui, p_model;
    ui_buffers b_ui{p_ui};
    model_buffers b_model{p_model};

    canvas(grid_viewport vp);
    void step(SDL_Event);
    void draw();

    void handle_mouse_motion(SDL_MouseMotionEvent motion)
    {
        auto [x, y] = Rxt::sdl::nds_coords(*window, motion.x, motion.y);
        auto gridpos = viewport.from_nds(x, y);

        selector.mouse_motion(gridpos);
    }

    void handle_mouse_down(SDL_MouseButtonEvent button);
    void handle_mouse_up(SDL_MouseButtonEvent button);
    void handle_should_quit() { quit = true; }
    void handle_key_down(SDL_Keysym k) { keys.press(k); }
    bool should_quit() const { return quit; }

    void _set_controls();
};
