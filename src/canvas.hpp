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

using grid_mouse_tool = mouse_tool<ivec>;

struct mouse_cursor : grid_mouse_tool//grid_mouse_tool
{
    ivec position {0};
    // uvec brush_size {1};
    const Rxt::rgba _color {0, 1, 1, .5};

    observable<ivec> hooks;

    void mouse_down(int) override { }
    void mouse_up(int) override { }

    void mouse_motion(ivec pos) override
    {
        position = pos;
        hooks.notify_all(position);
    }

    template <class Bufs>
    void update(Bufs& b)
    {
        b.push(position, uvec{1}, _color);
        b.update();
    }
};

struct mouse_select : grid_mouse_tool
{
    using region = std::tuple<ivec, ivec>;

    grid_viewport* viewport {};
    mouse_cursor* cursor;
    std::optional<ivec> drag_origin;
    std::optional<region> selection;
    Rxt::rgba _color = {1, 0, 1, 0.3};

    observable<mouse_select> hooks;

    mouse_select(grid_viewport* v, mouse_cursor* c) : viewport{v}, cursor{c} {}

    void mouse_down(int i) override
    {
        switch (i) {
        case 0:
            drag_origin = cursor->position;
            break;
        }
        hooks.notify_all(*this);
    }

    void mouse_up(int i) override
    {
        switch (i) {
        case 0:
            if (drag_origin) {
                auto [a, b] = Rxt::ordered(*drag_origin, cursor->position);
                selection = {a, b};
                drag_origin = {};
            }
            break;
        case 1:
            if (drag_origin) {
                drag_origin.reset();
            } else if (selection) {
                selection = {};
            }
            break;
        }
        hooks.notify_all(*this);
    }

    void mouse_motion(ivec pos) override
    {
        cursor->mouse_motion(pos);
        hooks.notify_all(*this);
    }

    template <class BCursor, class BSelection>
    void update(BCursor& bc, BSelection& bs)
    {
        uvec size{1};
        if (drag_origin) {
            auto [a, b] = Rxt::ordered(cursor->position, *drag_origin + viewport->position);
            size = b - a;
            bc.push(a, b - a, _color);
        } else {
            bc.push(cursor->position, size, _color);
        }
        bc.update();

        for (auto [a, b]: Rxt::to_range(selection)) {
            bs.push(a, b, _color);
        }
        bs.update();
    }
};

struct mouse_painter : grid_mouse_tool, observable<>
{
    mouse_cursor* cursor {};
    bool brush_down = false;
    uvec brush_size {1};

    mouse_painter(mouse_cursor* c) : cursor{c} {}

    void mouse_down(int) override { brush_down = true; }
    void mouse_up(int) override { brush_down = false; }
    void mouse_motion(ivec pos) override { if (brush_down) paint(cursor->position); }

    void paint(ivec pos) {}
};

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

struct canvas
    : Rxt::sdl::simple_gui
    , Rxt::sdl::input_handler<canvas>
{
    Rxt::sdl::key_dispatcher keys;
    bool enable_edge_scroll = true;
    bool quit = false;

    grid_viewport viewport;
    mouse_cursor cursor;
    mouse_select selector {&viewport, &cursor};
    mouse_painter painter {&cursor};
    grid_mouse_tool* tool;

    // display<grid_program> ui_disp, obj_disp;
    grid_program p_ui, p_obj;
    grid_program::data b_ui{p_ui}, b_obj{p_obj};

    canvas(grid_viewport vp);
    void step(SDL_Event);
    void draw();

    void handle_mouse_motion(SDL_MouseMotionEvent motion)
    {
        auto [x, y] = Rxt::sdl::nds_coords(*window, motion.x, motion.y);
        auto gridpos = floor(glm::vec2(x, y) * glm::vec2(viewport.size_cells() / 2u));

        if (tool) tool->mouse_motion(gridpos);
    }

    void handle_mouse_down(SDL_MouseButtonEvent button);
    void handle_mouse_up(SDL_MouseButtonEvent button);
    void handle_should_quit() { quit = true; }
    void handle_key_down(SDL_Keysym k) { keys.press(k); }
    bool should_quit() const { return quit; }
};
