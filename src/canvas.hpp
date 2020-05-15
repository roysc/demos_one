#pragma once

// #include "texture_display.hpp"
#include "gui/interface_display.hpp"

#include <Rxt/time.hpp>
#include <Rxt/util.hpp>
#include <Rxt/graphics/sdl.hpp>
#include <Rxt/graphics/gl.hpp>

#include <optional>
#include "observable.hpp"

namespace
{
struct mouse_tool
{
    virtual void mouse_down(ivec) = 0;
    virtual void mouse_down2(ivec) = 0;
    virtual void mouse_up(ivec) = 0;
    // virtual void mouse_motion(ivec) = 0;
};

struct empty_tool : mouse_tool
{
    void mouse_down(ivec) override {};
    void mouse_down2(ivec) override {};
    void mouse_up(ivec) override {};
    void mouse_motion(ivec) {};
};
}

struct mouse_select : mouse_tool, observable<mouse_select>
{
    using region = std::tuple<ivec, ivec>;
    std::optional<region> selection;
    std::optional<ivec> drag_origin;

    void mouse_down(ivec curpos) override
    {
        drag_origin = curpos;
        notify_observers(*this);
    }

    void mouse_down2(ivec) override
    {
        if (drag_origin) {
            drag_origin.reset();
        } else if (selection) {
            selection = {};
        }
        notify_observers(*this);
    }

    void mouse_up(ivec curpos) override
    {
        if (auto& origin = drag_origin) {
            auto [a, b] = Rxt::ordered(*origin, curpos);
            selection = {a, b};
            drag_origin = {};
        }
        notify_observers(*this);
    }
};

struct mouse_painter : mouse_tool, observable<>
{
    bool brush_down = false;
    uvec brush_size {1};

    void mouse_down(ivec) override { brush_down = true; }
    void mouse_down2(ivec) override { }
    void mouse_up(ivec) override { brush_down = false; }
    // void mouse_motion(ivec pos) { if (brush_down) paint(pos); }
 
    void paint(ivec pos) {}
};

struct cursor_state
{
    ivec position {0};
    uvec size {1};
    // inline static const Rxt::rgba color = {0, 1, 1, .5};
};

// template <class K>
struct canvas
    : Rxt::sdl::simple_gui
    , Rxt::sdl::input_handler<canvas>
{
    Rxt::sdl::key_dispatcher keys;
    bool enable_edge_scroll = true;
    bool quit = false;

    observable_value<grid_viewport> viewport;
    observable_value<cursor_state> cursor;

    using multi_tool = std::optional<mouse_tool*>;
    mouse_select selector;
    mouse_painter painter;
    multi_tool tool;

    interface_display ui;

    canvas(grid_viewport vp);
    void step(SDL_Event);
    void draw();

    void handle_mouse_motion(SDL_MouseMotionEvent motion)
    {
        auto [x, y] = Rxt::sdl::nds_coords(*window, motion.x, motion.y);
        auto gridpos = floor(glm::vec2(x, y) * glm::vec2(viewport.get().size_cells() / 2u));
        cursor.set({.position = gridpos});
        // tool->mouse_motion(cursor->position);
    }

    void handle_mouse_down(SDL_MouseButtonEvent button);
    void handle_mouse_up(SDL_MouseButtonEvent button);
    void handle_should_quit() { quit = true; }
    void handle_key_down(SDL_Keysym k) { keys.press(k); }
    bool should_quit() const { return quit; }
};

void canvas::step(SDL_Event event)
{
    do {
        handle_input(event);
    } while (SDL_PollEvent(&event));

    keys.scan();

    // Per-tick handlers
    if (enable_edge_scroll) {
        viewport.modify([&](auto& v) {v.edge_scroll(cursor->position, 1);});
    }

    if (is_dirty()) {
        draw();
        set_dirty(false);
    }
}

void canvas::draw()
{
    glClear(GL_COLOR_BUFFER_BIT);

    // b_features.draw();
    ui.draw(viewport.get());

    SDL_GL_SwapWindow(window.get());
}

void canvas::handle_mouse_down(SDL_MouseButtonEvent button)
{
    switch (button.button) {
    case SDL_BUTTON_LEFT: {
        if (tool) (*tool)->mouse_down(cursor->position);
        break;
    }
    case SDL_BUTTON_RIGHT: {
        if (tool) (*tool)->mouse_down2(cursor->position);
        break;
    }
    }
}

void canvas::handle_mouse_up(SDL_MouseButtonEvent button)
{
    switch (button.button) {
    case SDL_BUTTON_LEFT: {
        if (tool) (*tool)->mouse_up(cursor->position);
        break;
    }
    }
}
