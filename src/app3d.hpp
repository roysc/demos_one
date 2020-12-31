#pragma once

#include <Rxt/reactive.hpp>
#include <Rxt/controls.hpp>

#include <Rxt/graphics/sdl.hpp>
#include <Rxt/graphics/sdl/reactive.hpp>
#include <Rxt/graphics/camera.hpp>
#include <Rxt/graphics/shader/colored_triangle_3D.hpp>
#include <Rxt/graphics/shader/solid_color_3D.hpp>

#include <optional>

namespace sdl = Rxt::sdl;

// using panel_traits = spatial_traits<ivec, uvec>;
// using panel_viewport = basic_viewport<panel_traits>;
// using panel_layer = std::vector<std::pair<ivec, ivec>>; // todo index

struct basic_app3d : public sdl::simple_gui
{
    using triangle_program = Rxt::shader_programs::colored_triangle_3D;
    using line_program = Rxt::shader_programs::solid_color_3D<GL_LINES>;
    using point_program = Rxt::shader_programs::solid_color_3D<GL_POINTS>;

    using cursor_type = Rxt::adapt_reactive_crt<Rxt::reactive_cursor, Rxt::hooks<>, float>;
    using cursor_position_type = typename cursor_type::position_type;
    using viewport_uvec = Rxt::vec::uvec2;

    using camera_state = Rxt::focused_camera;
    using camera_type = Rxt::adapt_reactive_crt<Rxt::reactive_camera, Rxt::hooks<>, camera_state>;
    struct drag_state { cursor_position_type pos; camera_state cam; };

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

    basic_app3d(const char*, viewport_uvec);
    Rxt::reactive_handle _update(SDL_Event);
    bool is_stopped() const { return quit; }
    void draw();

    void _init_controls();
    void _init_ui();

    void reset_camera();
    void handle_drag(cursor_position_type, camera_state);
};
