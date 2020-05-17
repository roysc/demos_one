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

// struct mouse_painter : grid_mouse_tool, observable<>
// {
//     mouse_cursor* cursor {};
//     bool brush_down = false;
//     uvec brush_size {1};

//     mouse_painter(mouse_cursor* c) : cursor{c} {}

//     void mouse_down(int) override { brush_down = true; }
//     void mouse_up(int) override { brush_down = false; }
//     void mouse_motion(ivec pos) override { if (brush_down) paint(cursor->position); }

//     void paint(ivec pos) {}
// };

// template <class P>
// struct display
// {
//     using program_type = P;
//     using program_data = typename P::data;

//     P _prog;
//     program_data _bufs{_prog};
//     auto& program() {return _prog;}
//     auto& buffers() {return _bufs;}
// };

using grid_program = Rxt::shader_programs::webcompat::grid_quad_2D;

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
    mouse_cursor cursor;
    mouse_select selector {&viewport};
    grid_mouse_tool* cursor_tool;

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

        if (cursor_tool) cursor_tool->mouse_motion(gridpos);
    }

    void handle_mouse_down(SDL_MouseButtonEvent button);
    void handle_mouse_up(SDL_MouseButtonEvent button);
    void handle_should_quit() { quit = true; }
    void handle_key_down(SDL_Keysym k) { keys.press(k); }
    bool should_quit() const { return quit; }

    void _set_controls();
};
