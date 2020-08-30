#include "controls.hpp"
#include "input.hpp"

#include "reactive.hpp"
#include <Rxt/graphics/shader/colored_triangle_3D.hpp>
#include <Rxt/graphics/shader/solid_color_3D.hpp>
#include <Rxt/graphics/sdl.hpp>
#include <Rxt/graphics/camera.hpp>

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

    struct cursor_traits
    {
        using position_type = Rxt::fvec2;
    };
    using cursor_type = Rxt::adapt_reactive_crt<reactive_cursor, Rxt::hooks<>, cursor_traits>;
    using cursor_fvec = cursor_traits::position_type;
    using viewport_uvec = Rxt::uvec2;

    using camera_state = Rxt::focused_camera;
    using camera_type = Rxt::adapt_reactive_crt<reactive_cam, Rxt::hooks<>, camera_state>;
    struct drag_state { cursor_traits::position_type pos; camera_state cam; };

    bool quit = false;
    input_hooks input;
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
    void handle_drag(cursor_fvec, camera_state);
};
