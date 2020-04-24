#pragma once

#include "texture_grid.hpp"
#include "space.hpp"
#include "tiles.hpp"

#include <Rxt/runtime.hpp>
#include <Rxt/time.hpp>
#include <Rxt/util.hpp>
#include <Rxt/graphics/sdl.hpp>
#include <Rxt/graphics/gl.hpp>
#ifdef RXT_WEBGL2
  #include <Rxt/graphics/shader/webgl_grid_quad_2D.hpp>
#else
  #include <Rxt/graphics/shader/grid_quad_2D.hpp>
#endif

#include <functional>
#include <optional>
#include <variant>
#include <chrono>

template <class T>
using grid_map = boost::multi_array<T, 2>;

using std::chrono::steady_clock;
using time_point = steady_clock::time_point;

struct selection_tool
{
    using region = std::tuple<ivec, uvec>;
    std::optional<region> selection;
    std::optional<ivec> drag_origin;
};

struct pen_tool
{
    bool down = false;
    tile_id ink_fg = 1;
};

struct map_editor
    : public virtual grid_view
    , public virtual Rxt::sdl::simple_gui
    , public virtual texture_grid
    , public virtual Rxt::simple_runtime
{
    using tick_duration = Rxt::duration_fps<30>;
    using tool_state = std::variant<std::monostate, selection_tool, pen_tool>;

#ifdef RXT_WEBGL2
    using grid_program = Rxt::shader_programs::webgl::grid_quad_2D;
#else
    using grid_program = Rxt::shader_programs::grid_quad_2D;
#endif

    grid_program quad_prog{};
    grid_program::data b_quads {quad_prog};
    grid_program::data b_quads_sticky {quad_prog}; // for cursor
    grid_program::data b_features {quad_prog}; // static

    Rxt::sdl::key_dispatcher keys;
#ifndef __EMSCRIPTEN__
    Rxt::sdl::metronome metronome{tick_duration{1}, [this] { return !should_quit(); }};
#endif
    time_point t_last = steady_clock::now();

    Rxt::lazy_action update_features, update_cursor, update_tool, update_viewport;

    const Rxt::rgba cursor_color {0, 1, 1, .5};
    ivec cursor_position {0}; // relative to viewport

    bool p_edge_scroll = true;
    tool_state current_tool;
    grid_map<tile_id> grid_layer;

    map_editor(int);

    template <class M>
    M* get_tool() { return std::get_if<M>(&current_tool); }

    void step(SDL_Event);
    void draw();

    // void _update_entities(tick_duration);
    void _update_features();
    void _update_cursor();
    void _update_tool();
    void _update_viewport();

    void handle(SDL_Event);
    void h_mouse_motion(SDL_MouseMotionEvent);
    void h_mouse_down(SDL_MouseButtonEvent);
    void h_mouse_up(SDL_MouseButtonEvent);
    void h_edge_scroll();
};
