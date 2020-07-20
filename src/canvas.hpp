#pragma once

#include "map.hpp"
#include "mouse_tools.hpp"
#include "controls.hpp"
#include "reactive.hpp"
#include "input.hpp"

#include <Rxt/graphics/sdl.hpp>
#include <Rxt/graphics/shader/grid_quad_2D.hpp>
#include <Rxt/graphics/shader/solid_color_3D.hpp>

#include <array>

using Rxt::hooks;
using grid_program = Rxt::shader_programs::webcompat::grid_quad_2D;

struct grid_traits
{
    using position_type = ivec;
    using size_type = uvec;
};

using grid_controls = controls_2d<grid_traits>;
using grid_selector = mouse_select_tool<grid_traits>;
using grid_painter = mouse_paint_tool<grid_traits>;
using stroke_tool = mouse_stroke_tool<grid_traits>;

using viewport_type = Rxt::adapt_reactive_crt<reactive_viewport, hooks<>, grid_traits>;
using cursor_type = Rxt::adapt_reactive_crt<reactive_cursor, hooks<>, grid_traits>;

struct tool_hooks
{
    hooks<> on_viewport_update;
    hooks<> on_cursor_update;
    hooks<> on_edit;
    hooks<> on_select;
    hooks<> on_debug;

    static constexpr auto members()
    {
        return std::array{
            (&tool_hooks::on_viewport_update),
            (&tool_hooks::on_cursor_update),
            (&tool_hooks::on_edit),
            (&tool_hooks::on_select),
            (&tool_hooks::on_debug)
        };
    }
};

using tool_router = Rxt::hook_router<mouse_tool*, tool_hooks>;

using glm::fvec2;
using line_program = Rxt::shader_programs::solid_color_3D<GL_LINES>;
using Rxt::rgba;

struct model_buffers : grid_program::buffers
{
    const rgba select_color {0, 0, 1, 1};

    void add_selection(ivec a, ivec b)
    {
        push(a, b-a+1, select_color);
    }
};

struct ui_buffers : grid_program::buffers
{
    void set_cursor(position_vec p, size_vec s, rgba color)
    {
        clear();
        push(p, s, color);
        update();
    }
};

struct line_buffers
    : line_program::buffers
{
    void add_line(ivec a, ivec b, rgba color)
    {
        push(position_vec(a, 0), color);
        push(position_vec(b, 0), color);
    }
};

struct canvas
    : Rxt::sdl::simple_gui
{
    Rxt::sdl::key_dispatcher keys;
    input_hooks input;
    bool enable_edge_scroll = true;
    bool quit = false;

    viewport_type viewport;
    cursor_type cursor;
    grid_controls controls{cursor, viewport};

    grid_selector selector {controls};
    stroke_tool stroker {controls};
    grid_painter painter {controls};
    mouse_tool* tool {};
    tool_router router;

    grid_program p_ui, p_quad;
    line_program p_lines;
    ui_buffers b_ui{p_ui};
    model_buffers b_model{p_quad};
    model_buffers b_paint{p_quad};
    line_buffers b_lines{p_lines};
    line_buffers b_lines_cursor{p_lines};

    // interesting stuff
    using grid_map = dense_grid<int>;
    grid_map paint_layer;

    template <class V>
    canvas(V vp)
        : simple_gui("plaza: canvas", vp.size_pixels())
        , viewport(vp)
    {
        _init_observers();
        _init_controls();
        glClearColor(0, 0, 0, 1);
    }

    void advance(SDL_Event);
    void draw();

    bool is_stopped() const { return quit; }

    void _init_controls();
    void _init_observers();
};
