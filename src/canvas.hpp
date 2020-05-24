#pragma once

#include "tool.hpp"
#include "mouse.hpp"
#include "map.hpp"

#include <Rxt/graphics/sdl.hpp>
#include <Rxt/graphics/shader/grid_quad_2D.hpp>
#include <Rxt/graphics/shader/solid_color_3D.hpp>

struct grid_traits
{
    using position_type = ivec;
    using size_type = uvec;
};

using grid_program = Rxt::shader_programs::webcompat::grid_quad_2D;
using grid_viewport = viewport<grid_traits>;

// using ui_traits = mouse_ui<grid_traits>;
using grid_controls = control_port<grid_traits>;
using grid_selector = mouse_select_tool<grid_traits>;
using grid_painter = mouse_paint_tool<grid_traits>;

using stroke_tool = mouse_stroke_tool<grid_traits>;
using multi_tool = swappable_tool<
    tags::viewport_tag,
    tags::cursor_motion_tag,
    tags::debug_tag
    >;

using glm::fvec2;
using line_program = Rxt::shader_programs::solid_color_3D<GL_LINES>;

using tag_router = observer_router<
    tags::viewport_tag,
    tags::cursor_motion_tag,
    tags::cursor_selection_tag,
    tags::object_edit_tag
    >;
// using tag_router = subject_router;

using Rxt::rgba;

struct model_buffers : grid_program::data
{
    const rgba select_color {0, 0, 1, 1};

    void add_selection(ivec a, ivec b)
    {
        clear();
        push(a, b-a+1, select_color);
        update();
    }
};

struct ui_buffers : grid_program::data
{
    void set_cursor(position_vec p, size_vec s, rgba color)
    {
        clear();
        push(p, s, color);
        update();
    }
};

struct line_buffers
    : line_program::data
{
    void add_line(ivec a, ivec b, rgba color)
    {
        push(position_vec(a, 0), color);
        push(position_vec(b, 0), color);
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

    tag_router obr;

    grid_controls controls;
    grid_viewport& viewport = controls.viewport();

    grid_selector selector {controls};
    stroke_tool stroker {controls};
    grid_painter painter {controls};
    multi_tool tool;

    grid_program p_ui, p_model;
    ui_buffers b_ui{p_ui};
    model_buffers b_model{p_model};
    model_buffers b_paint{p_model};

    line_program p_lines;
    line_buffers b_lines{p_lines};
    line_buffers b_lines_cursor{p_lines};

    // interesting stuff
    using grid_map = array2_map<int>;
    grid_map paint_layer;

    canvas(grid_viewport vp);
    void step(SDL_Event);
    void draw();

    void handle_mouse_motion(SDL_MouseMotionEvent motion)
    {
        auto [x, y] = Rxt::sdl::nds_coords(*window, motion.x, motion.y);
        auto gridpos = controls.viewport().from_nds(x, y);

        controls.cursor_position(gridpos);
    }

    void handle_mouse_down(SDL_MouseButtonEvent button);
    void handle_mouse_up(SDL_MouseButtonEvent button);
    void handle_should_quit() { quit = true; }
    void handle_key_down(SDL_Keysym k) { keys.press(k); }
    bool should_quit() const { return quit; }

    void _set_controls();
};
