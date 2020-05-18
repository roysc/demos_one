#pragma once

// #include "texture_display.hpp"
#include "gui/interface_display.hpp"

#include <Rxt/time.hpp>
#include <Rxt/range.hpp>
#include <Rxt/util.hpp>
#include <Rxt/graphics/sdl.hpp>
#include <Rxt/graphics/gl.hpp>

#include <optional>
#include "observable.hpp"
#include "mouse.hpp"

using grid_program = Rxt::shader_programs::webcompat::grid_quad_2D;
using grid_mouse = mouse_tool<ivec>;
// using grid_cursor = mouse_cursor_tool<ivec>;
using grid_select = mouse_select_tool<ivec>;

struct ent_buffers : grid_program::data
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
    // grid_program _p;
    // grid_program& get_program() { return _p; }
    // variant<grid_program*, grid_program> _pp;
    // grid_program& get_program()
    // {
    //     auto v = overload {[](grid_program* p) {return *p;}, [](grid_program& p) {return p;}};
    //     return visit(v, _pp);
    // }
    // grid_program::uniforms* operator->() { return &get_program().u_; }

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

    grid_viewport viewport;
    grid_select cursor {&viewport};
    grid_mouse* mouse_tool {&cursor};

    grid_program p_ui, p_obj;
    ui_buffers b_ui{p_ui};
    ent_buffers b_obj{p_obj};

    canvas(grid_viewport vp);
    void step(SDL_Event);
    void draw();

    void handle_mouse_motion(SDL_MouseMotionEvent motion)
    {
        auto [x, y] = Rxt::sdl::nds_coords(*window, motion.x, motion.y);
        auto gridpos = viewport.from_nds(x, y);

        cursor.mouse_motion(gridpos);
    }

    void handle_mouse_down(SDL_MouseButtonEvent button);
    void handle_mouse_up(SDL_MouseButtonEvent button);
    void handle_should_quit() { quit = true; }
    void handle_key_down(SDL_Keysym k) { keys.press(k); }
    bool should_quit() const { return quit; }

    void _set_controls();
};
