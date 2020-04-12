#pragma once

#include "space.hpp"
#include "grid_display.hpp"
// #include "agent.hpp"
#include "tiles.hpp"

#include <Rxt/runtime.hpp>
#include <Rxt/time.hpp>
#include <Rxt/util.hpp>

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
    using region = std::tuple<grid_coord, grid_size>;
    std::optional<region> selection;
    std::optional<grid_coord> drag_origin;
};

struct pen_tool
{
    bool down = false;
    tile_id ink_fg = 1;
};

struct map_editor : public grid_display
                  , public virtual Rxt::simple_runtime
{
    using tick_duration = Rxt::duration_fps<30>;
    using tool_state = std::variant<std::monostate, selection_tool, pen_tool>;

    static constexpr grid_size world_size {64};
    static constexpr grid_size tile_size_px {8};

    Rxt::sdl::key_dispatcher keys;
    std::thread metronome;
    time_point t_last = steady_clock::now();

    Rxt::lazy_action update_features, update_cursor, update_tool;

    grid_coord cursor_position {0}; // relative to viewport
    grid_program::data b_quads {quad_prog};
    grid_program::data b_quads_sticky {quad_prog}; // for cursor
    // grid_program::data b_mobile_entities {quad_prog};
    grid_program::data b_features {quad_prog}; // static

    bool p_edge_scroll = true;
    tool_state current_tool;

    // entt::registry registry;
    // std::function<void()> move_entities;
    grid_map<tile_id> grid_layer;

    map_editor(int);
    ~map_editor() override { metronome.join(); }

    template <class M>
    M* get_tool() { return std::get_if<M>(&current_tool); }

    void step();
    void draw();

    // void _update_entities(tick_duration);
    void _update_features();
    void _update_cursor();
    void _update_tool();

    void handle(SDL_Event);
    void h_mouse_motion(SDL_MouseMotionEvent);
    void h_mouse_down(SDL_MouseButtonEvent);
    void h_mouse_up(SDL_MouseButtonEvent);
    void h_edge_scroll();
};
