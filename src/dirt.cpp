#pragma once

#include <Rxt/graphics/shader/colored_triangle_3D.hpp>
// #include <Rxt/graphics/shader/solid_color_3D.hpp>

// #include <Rxt/geometry/shapes.hpp>

#include <Rxt/graphics/sdl.hpp>
#include <Rxt/graphics/gl.hpp>
#include <Rxt/graphics/camera.hpp>
// #include <Rxt/time.hpp>
// #include <Rxt/util.hpp>

#include <glm/glm.hpp>

// #include <chrono>
// #include <thread>
// #include <utility>

namespace gl = Rxt::gl;
namespace sdl = Rxt::sdl;
using triangle_program = Rxt::shader_programs::colored_triangle_3D;
// using line_program = Rxt::shader_programs::solid_color_3D<GL_LINES>;

using std::chrono::steady_clock;
using time_point = steady_clock::time_point;


struct float_traits {
    using position_type = glm::fvec2;
    using size_type = glm::fvec2;
};

using float_controls = control_port<Ftr>;

struct dirt_app : public sdl::simple_gui
                    , public sdl::input_handler<dirt_app>
{
    gl::program_loader loader;
    Rxt::focused_camera camera {camera_start()};
    sdl::key_dispatcher keys;

    triangle_program triangle_prog {loader};
    triangle_program::data b_triangles {triangle_prog};

    line_program line_prog {loader};
    line_program::data b_lines {line_prog};

    mesh_data geom;
    mesh_colors colors;
    ux_data ux;
    std::vector<std::pair<glm::vec3, glm::vec3>> dbg_segments;

    glm::vec2 cursor_position;
    time_point last_render_time;
    sdl::metronome metronome {Rxt::duration_fps<30>(1), [this] { return !should_quit(); }};

    dirt_app();

    void init_controls();
    void set_cursor(float x, float y)
    {
        cursor_position = {x, y};
        update_cursor();
    }

    object_index insert_mesh(object_mesh mesh, Rxt::rgb color)
    {
        auto index = geom.insert(mesh);
        colors.emplace(index, color);
        return index;
    }

    void step(SDL_Event);
    void draw();

    void handle(SDL_Event);
    void h_mouse_motion(SDL_MouseMotionEvent);
    void h_mouse_down(SDL_MouseButtonEvent);
    void h_mouse_up(SDL_MouseButtonEvent);

    static glm::vec3 camera_start() { return glm::vec3 {1}; }
};
