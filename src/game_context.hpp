#pragma once

#include "space.hpp"
#include "grid_context.hpp"
#include "agent.hpp"
#include "tiles.hpp"

#include <Rxt/graphics/sdl.hpp>
#include <Rxt/graphics/gl.hpp>
#include <Rxt/graphics/color.hpp>
#include <Rxt/graphics/glm.hpp>
#include <Rxt/runtime.hpp>
#include <Rxt/time.hpp>
#include <Rxt/util.hpp>

#include <entt/entt.hpp>
#include <glm/gtx/transform.hpp>

#include <functional>
#include <optional>
#include <variant>
#include <chrono>
#include <random>
#include <iterator>

using std::optional;
using std::get_if;

using glm::ivec2;
using glm::vec4;

namespace sdl = Rxt::sdl;
namespace gl = Rxt::gl;
using Rxt::dbg::print;

using std::chrono::steady_clock;
using time_point = steady_clock::time_point;

// constexpr float epsilon = 1e-2;

struct selection_tool
{
    using region = std::tuple<grid_coord, grid_size>;
    optional<region> selection;
    optional<grid_coord> drag_origin;
};

struct pen_tool
{
    bool down = false;
    tile_id ink_fg = 1;
};

struct game_context : public grid_context
                    , public virtual Rxt::simple_runtime
{
    using tick_duration = Rxt::duration_fps<30>;

    static constexpr grid_size world_size {64};
    static constexpr glm::uvec2 tile_size_px {8};

    sdl::key_dispatcher keys;
    std::thread metronome;
    time_point t_last = steady_clock::now();

    grid_coord cursor_position {0}; // relative to viewport
    grid_quad_2D::data b_quads_sticky {quad_prog}; // for cursor
    grid_quad_2D::data b_mobile_entities {quad_prog};
    grid_quad_2D::data b_immobile_entities {quad_prog}; // static

    bool p_edge_scroll = true;

    std::variant<std::monostate,
                 selection_tool,
                 pen_tool> current_tool;

    entt::registry registry;
    // std::function<void()> move_entities;
    grid_map<tile_id> grid_layer;

    game_context(int);
    ~game_context() override { metronome.join(); }

    template <class M>
    M* get_tool() { return get_if<M>(&current_tool); }

    void step();
    void draw();

    void update_entities(tick_duration);
    void update_features();

    void update_cursor();
    void update_selection();

    void handle(SDL_Event);
    void h_mouse_motion(SDL_MouseMotionEvent);
    void h_mouse_down(SDL_MouseButtonEvent);
    void h_mouse_up(SDL_MouseButtonEvent);
    void h_edge_scroll();
};
