#pragma once

#include "space.hpp"
#include "grid_context.hpp"
#include "agent.hpp"
#include "tiles.hpp"

#include <Rxt/graphics/sdl.hpp>
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
using Rxt::grid_quad;

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
                    , public Rxt::simple_runtime
{
    using tick_duration = Rxt::duration_fps<30>;

    static constexpr grid_size world_size {64};
    static constexpr glm::uvec2 tile_size_px {8};

    sdl::key_dispatcher keys;
    std::thread metronome;
    time_point t_last = steady_clock::now();

    grid_coord cursor_position {0}; // relative to viewport
    grid_quad b_quads_sticky {quad_prog}; // for cursor
    grid_quad b_mobile_entities {quad_prog};
    grid_quad b_immobile_entities {quad_prog}; // static

    bool p_edge_scroll = false;

    std::variant<std::monostate,
                 selection_tool,
                 pen_tool> current_tool;

    entt::registry registry;
    // std::function<void()> move_entities;
    grid_map<tile_id> grid_layer;

    game_context();
    ~game_context() override { metronome.join(); }

    template <class M>
    M* get_tool() { return get_if<M>(&current_tool); }

    void step();
    void draw();

    void render_entities(tick_duration);
    void render_features();

    void render_cursor();
    void render_selection();

    void handle(SDL_Event);
    void h_mouse_motion(SDL_MouseMotionEvent);
    void h_mouse_down(SDL_MouseButtonEvent);
    void h_mouse_up(SDL_MouseButtonEvent);
    void h_edge_scroll();
};

game_context::game_context()
    : grid_context {"demo", world_size, tile_size_px}
    , grid_layer (boost::extents[world_size[0]][world_size[1]])
{
    keys.on_press["C-W"]    = [this] { should_quit(true); };
    keys.on_press["C"]      = [this] {
        viewport_position = grid_coord {0};
        render_viewport();
    };
    keys.on_press["Escape"] = [this] {
        current_tool = {};
        render_viewport();
        render_selection();
        print("Disabled modes\n");
    };

    keys.on_press["E"] = [this] { p_edge_scroll = !p_edge_scroll; };
    keys.on_press["S"] = [this] { current_tool = selection_tool {}; print("Tool: select\n"); };
    keys.on_press["B"] = [this] { current_tool = pen_tool {}; print("Tool: pen\n"); };

    keys.on_press["I"] = [this] {
        print("pos=({0:+}, {1:+}) ", viewport_position.x, viewport_position.y);
        print("size=({0}, {1}) ", viewport_size.x, viewport_size.y);
        print("cursor=({0:+}, {1:+})\n", cursor_position.x, cursor_position.y);
        if (auto select_mode = get_tool<selection_tool>();
            select_mode && select_mode->selection) {
            auto [a, b] = *select_mode->selection;
            print("selected: pos=({}, {}) size=({}, {})\n", a.x, a.y, b.x, b.y);
        }
    };

    keys.on_scan["Left"]  = [this] { h_move_viewport(-1, 0); };
    keys.on_scan["Right"] = [this] { h_move_viewport(+1, 0); };
    keys.on_scan["Down"]  = [this] { h_move_viewport(0, -1); };
    keys.on_scan["Up"]    = [this] { h_move_viewport(0, +1); };
    keys.on_press["."] = std::bind(&grid_context::h_scale_viewport, this, +1);
    keys.on_press[","] = std::bind(&grid_context::h_scale_viewport, this, -1);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    auto image = create_map(world_size, 42);
    render_texture(data(image));

    // initialize cursor
    b_quads_sticky.position.storage = {cursor_position};
    b_quads_sticky.color.storage = {vec4(0, 1, 1, .5)};
    b_quads_sticky.size.storage = {vec2(1)};
    b_quads_sticky.update();

    render_model();
    render_viewport();

    // glClearColor(0, 0.69, 0, 1);
    auto russet = Rxt::rgb_hex(0x80461B);
    glClearColor(russet.r, russet.g, russet.b, 1);

    metronome = sdl::start_metronome(tick_duration {1}, [this] { return !should_quit(); });

    auto e = create_agent(registry, true, grid_coord {-1,0});
}

void game_context::step()
{
    SDL_Event event;
    SDL_WaitEvent(&event);
    do {
        handle(event);
    } while (SDL_PollEvent(&event));

    keys.scan();

    if (p_edge_scroll) {
        h_edge_scroll();
    }

    {
        auto now = steady_clock::now();
        tick_duration dt = now - t_last;
        // if more than one tick has passed, render
        if (dt.count() > 1) {
            for (int i = 0; i < dt.count(); ++i) {
                // move_entities();
            }
            render_entities(dt);
            t_last = now;
        }
    }

    if (is_dirty()) {
        draw();
        set_dirty(false);
    }
}

void game_context::draw()
{
    glClear(GL_COLOR_BUFFER_BIT);

    b_texs.draw();
    b_mobile_entities.draw();
    b_immobile_entities.draw();
    b_quads.draw();

    { // todo: use UBOs and ubo_guard
        gl::uniform<grid_coord> u_vpos {quad_prog, "viewportPosition"};
        set(u_vpos, grid_coord {0});
        b_quads_sticky.draw();
        set(u_vpos, viewport_position);
    }

    SDL_GL_SwapWindow(window.get());
}

void game_context::render_features()
{
    b_immobile_entities.clear();

    auto shape = grid_layer.shape();
    for (unsigned y = 0; y < shape[1]; ++y) {
        for (unsigned x = 0; x < shape[0]; ++x) {
            auto cell = grid_layer[x][y];
            if (cell != 0) {
                auto tile = tiles.at(cell);
                b_immobile_entities.push(grid_coord {x, y}, grid_size {1}, Rxt::rgba(tile.color, 1));
            }
        }
    }
    b_immobile_entities.update();

    set_dirty();
}

void game_context::render_entities(tick_duration dt)
{
    b_mobile_entities.clear();

    auto view = registry.view<agent_position>();
    for (auto e: view) {
        auto& pos = view.get<agent_position>(e);
        b_mobile_entities.push(pos.coord, grid_size {1}, Rxt::rgba(0, 1, 0, 1));
    }
    b_mobile_entities.update();

    set_dirty();
}

void game_context::render_selection()
{
    // render selected regions
    b_quads.clear();
    if (auto select = get_tool<selection_tool>()) {
        for (auto [a, b]: Rxt::to_range(select->selection)) {
            b_quads.push(a, b, Rxt::rgba(Rxt::colors::hot_pink, 0.5)); //todo
        }
    }
    b_quads.update();

    set_dirty();
}

void game_context::render_cursor()
{
    b_quads_sticky.position.storage[0] = cursor_position;
    b_quads_sticky.size.storage[0] = grid_size(1);

    auto visitor = Rxt::overloaded {
        [this] (selection_tool& select) {
            // render cursor drag area
            if (auto& origin = select.drag_origin) {
                auto [a, b] = ordered(*origin, cursor_position);
                b_quads_sticky.position.storage[0] = a;
                b_quads_sticky.size.storage[0] = b - a + 1;
            }
        },
        [this] (pen_tool& pen) {
            if (pen.down) {
                grid_coord pos = cursor_position + viewport_position;
                pos.x %= world_size.x;
                pos.y %= world_size.y;
                grid_layer[pos.x][pos.y] = pen.ink_fg;
                // print("pen: {} at ({}, {})\n", tiles.at(pen.ink_fg).description, pos.x, pos.y);
                render_features();
            }
        },
        [] (auto) {}
    };
    visit(visitor, current_tool);

    b_quads_sticky.update();

    set_dirty();
}

void game_context::h_mouse_motion(SDL_MouseMotionEvent motion)
{
    auto [x, y] = sdl::nds_coords(*window, motion.x, motion.y);
    cursor_position = nds_to_grid(vec2(x, y), vec2(viewport_size / 2u));
    render_cursor();
}

void game_context::h_mouse_down(SDL_MouseButtonEvent button)
{
    auto left_visitor = Rxt::overloaded {
        [this] (selection_tool& select) {
            select.drag_origin = cursor_position;
            render_cursor();
        },
        [this] (pen_tool& pen) {
            pen.down = true;
            render_cursor();
        },
        [] (auto) {}
    };

    auto right_visitor = Rxt::overloaded {
        // Right-click while selecting grid cancels
        [this] (selection_tool& select) {
            if (select.drag_origin) {
                select.drag_origin.reset();
                render_cursor();
            } else if (select.selection) {
                select.selection = {};
                render_selection();
            }
        },
        [] (auto) {}
    };

    switch (button.button) {
    case SDL_BUTTON_LEFT: {
        visit(left_visitor, current_tool);
        break;
    }
    case SDL_BUTTON_RIGHT: {
        visit(right_visitor, current_tool);
        break;
    }
    }
}

void game_context::h_mouse_up(SDL_MouseButtonEvent button)
{
    switch (button.button) {
    case SDL_BUTTON_LEFT: {
        auto visitor = Rxt::overloaded {
            [this] (selection_tool& select) {
                if (auto& origin = select.drag_origin) {
                    auto [a, b] = ordered(*origin, cursor_position);
                    select.selection.emplace(a + viewport_position, b - a + 1);
                    select.drag_origin = {};
                    render_cursor();
                    render_selection();
                }
            },
            [this] (pen_tool& pen) {
                pen.down = false;
            },
            [] (auto) {}
        };
        visit(visitor, current_tool);
        break;
    }
    }
}

// edge-of-screen cursor scrolling
void game_context::h_edge_scroll()
{
    // (0,0) is center-screen, so offset it to the corner
    ivec2 offset_pos = cursor_position + ivec2(viewport_size / 2u);
    ivec2 dv {0};
    for (unsigned i = 0; i < dv.length(); ++i) {
        if (offset_pos[i] == 0) {
            dv[i] = -1;
        } else if (offset_pos[i] + 1 == viewport_size[i]) {
            dv[i] = +1;
        }
    }
    if (dv != ivec2 {0}) {
        h_move_viewport(dv.x, dv.y);
    }
}

void game_context::handle(SDL_Event event)
{
    switch (event.type) {
    case SDL_QUIT: { should_quit(true); return; }
    case SDL_KEYDOWN: { keys.press(event.key.keysym); break; }
    case SDL_MOUSEMOTION: { h_mouse_motion(event.motion); break; }
    case SDL_MOUSEBUTTONDOWN: { h_mouse_down(event.button); break; }
    case SDL_MOUSEBUTTONUP: { h_mouse_up(event.button); break; }
    }
}
