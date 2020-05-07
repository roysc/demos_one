#pragma once

#include "space.hpp"
#include "tiles.hpp"

#include "texture_display.hpp"
#include "interface_display.hpp"

#include <Rxt/time.hpp>
#include <Rxt/util.hpp>
#include <Rxt/graphics/sdl.hpp>
#include <Rxt/graphics/gl.hpp>

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
    : public virtual Rxt::sdl::simple_gui
    , public virtual Rxt::simple_runtime
{
    using tick_duration = Rxt::duration_fps<30>;
    using tool_state = std::variant<std::monostate, selection_tool, pen_tool>;

    uvec grid_size;
    grid_viewport viewport;
    ivec cursor_position {0}; // relative to viewport

    bool enable_edge_scroll = true;
    tool_state current_tool;
    grid_map<tile_id> grid_layer;

    texture_display background;
    interface_display interface;
    interface_display::grid_program::data b_features {interface.quad_prog}; // static

    Rxt::sdl::key_dispatcher keys;
    Rxt::sdl::metronome metronome{tick_duration{1}, [this] { return !should_quit(); }};
    time_point t_last = steady_clock::now();

    Rxt::lazy_action update_features, update_cursor, update_tool, update_viewport;

    map_editor(int, uvec, grid_viewport);
    map_editor(int seed, uvec size)
        : map_editor(seed, size, grid_viewport{size, uvec{8}})
    {}

    template <class M>
    bool get_tool(M& out)
    {
        M* ret = std::get_if<M>(&current_tool);
        if (ret) out = *ret;
        return ret;
    }

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
