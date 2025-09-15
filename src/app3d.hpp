#pragma once

#include <Rxt/camera.hpp>
#include <Rxt/controls.hpp>
#include <Rxt/graphics/sdl.hpp>
#include <Rxt/graphics/sdl/reactive.hpp>
#include <Rxt/graphics/shaders/colored_triangle_3D.hpp>
#include <Rxt/graphics/shaders/solid_color_3D.hpp>
#include <Rxt/reactive.hpp>

#include <optional>

namespace sdl = Rxt::sdl;

// using panel_traits = spatial_traits<ivec, uvec>;
// using panel_viewport = basic_viewport<panel_traits>;
// using panel_layer = std::vector<std::pair<ivec, ivec>>; // todo index

struct basic_app3d : public sdl::simple_gui
{
    using triangle_program = Rxt::shaders::colored_triangle_3D;
    using line_program = Rxt::shaders::solid_color_3D<GL_LINES>;
    using point_program = Rxt::shaders::solid_color_3D<GL_POINTS>;

    using cursor_type = Rxt::reactive_cursor<float>;
    using cursor_position_type = cursor_type::position_type;
    using viewport_size_type = Rxt::vec::uvec2;

    using camera_state = Rxt::focused_camera;
    using camera_type = Rxt::reactive_camera<camera_state>;
    struct drag_state
    {
        cursor_position_type pos;
        camera_state cam;
    };

    bool quit = false;
    Rxt::sdl::input_hooks input;
    sdl::key_dispatcher _keys;
    auto& keys() { return _keys; }

    camera_state initial_camera;
    camera_type camera;
    cursor_type cursor;
    std::optional<drag_state> drag_origin;

    // panel_viewport ui_viewport;
    // panel_layer ui_objects;

    triangle_program triangle_prog;
    line_program line_prog, ui_line_prog;
    point_program point_prog;

    Rxt::hooks<> on_debug;

    basic_app3d(const char*, viewport_size_type);
    Rxt::hooks<> _updates(SDL_Event);
    bool running() const { return !quit; }
    void draw();

    void _init_controls();
    void _init_ui();

    void reset_camera();
    void handle_drag(cursor_position_type, camera_state);
};
