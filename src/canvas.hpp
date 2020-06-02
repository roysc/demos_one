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
using grid_controls = control_port<grid_traits>;
using grid_selector = mouse_select_tool<grid_traits>;
using grid_painter = mouse_paint_tool<grid_traits>;
using stroke_tool = mouse_stroke_tool<grid_traits>;

using tool_tags = Rxt::type_tuple<
    tags::debug_tag,
    tags::viewport_tag,
    tags::cursor_motion_tag,
    tags::object_edit_tag,
    tags::cursor_selection_tag
    >;

using main_router = Rxt::tuple_apply_t<observer_router, tool_tags>;
using main_tool = Rxt::tuple_apply_t<swappable_tool, tool_tags>;

// The hook list implementation routed to by the swappable_tool
// uses eager_observable<Tag> for the tool's exposed tags
using tool_observable = Rxt::tuple_apply_t<multi_observable,
    Rxt::tuple_map_t<eager_observable, main_tool::observable_tags>>;

template <class Tool>
using tool_proxy = observable_proxy<Tool, tool_observable>;

using glm::fvec2;
using line_program = Rxt::shader_programs::solid_color_3D<GL_LINES>;
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
    }
};

struct canvas
    : Rxt::sdl::simple_gui
    , Rxt::sdl::input_handler<canvas>
{
    Rxt::sdl::key_dispatcher keys;
    bool enable_edge_scroll = true;
    bool quit = false;

    main_router router;

    grid_controls controls;
    grid_viewport& viewport = controls.viewport();

    tool_proxy<grid_selector> selector {controls};
    tool_proxy<stroke_tool> stroker {controls};
    tool_proxy<grid_painter> painter {controls};

    main_tool tool;
    tool_observable tool_hooks;

    grid_program p_ui, p_quad;
    line_program p_lines;

    ui_buffers b_ui{p_ui};
    model_buffers b_model{p_quad};
    model_buffers b_paint{p_quad};
    line_buffers b_lines{p_lines};
    line_buffers b_lines_cursor{p_lines};

    // interesting stuff
    using grid_map = dense_map<int>;
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
