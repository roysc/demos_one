#include "controls.hpp"
#include "reactive.hpp"
#include "map.hpp"
#include "input.hpp"

#include "atrium/geometry.hpp"
#include "atrium/rendering.hpp"
#include "atrium/interaction.hpp"

#include <Rxt/graphics/shader/colored_triangle_3D.hpp>
#include <Rxt/graphics/shader/solid_color_3D.hpp>
#include <Rxt/graphics/sdl.hpp>
#include <Rxt/graphics/gl.hpp>
#include <Rxt/graphics/camera.hpp>

#include <Rxt/math.hpp>
#include <Rxt/time.hpp>
#include <Rxt/util.hpp>

#include <glm/glm.hpp>
#include <chrono>
#include <vector>

using std::chrono::steady_clock;
using time_point = steady_clock::time_point;

namespace gl = Rxt::gl;
namespace sdl = Rxt::sdl;
using triangle_program = Rxt::shader_programs::colored_triangle_3D;
using line_program = Rxt::shader_programs::solid_color_3D<GL_LINES>;

using uvec2 = glm::uvec2;
using fvec2 = glm::vec2;
using fvec3 = glm::vec3;

struct ui_traits
{
    using position_type = fvec2;
    using size_type = uvec2;
};

using namespace Rxt::frp;
using cursor_type = adapt_reactive_crt<reactive_cursor, hooks<>, ui_traits>;
using camera_type = adapt_reactive_crt<reactive_focus_cam, hooks<>>;

using atrium::mesh_data;
using atrium::mesh_colors;
using atrium::object_mesh;
using ux_data = adapt_reactive<atrium::ux_data>;

//wip
// using agent_registry = std::vector<int>;
using int8 = unsigned char;
using terrain_value = int8;
using terrain_map = dense_map<terrain_value>;

using tool_router = Rxt::hook_router<int, input_hooks>;

struct dirt_app : public sdl::simple_gui
{
    sdl::key_dispatcher keys;
    input_hooks input;
    bool quit = false;
    sdl::metronome metronome {Rxt::duration_fps<30>(1), [this] { return !is_stopped(*this); }};
    // time_point last_draw_time;
    // bool draw_needed = true;

    fvec3 const start_camera_at{8};
    camera_type camera{start_camera_at};
    cursor_type cursor;

    triangle_program triangle_prog;
    triangle_program::data b_triangles {triangle_prog};
    line_program line_prog;
    line_program::data b_lines {line_prog};

    mesh_data geom;
    mesh_colors colors;
    ux_data ux;
    hooks<> model_update;

    adapt_reactive<terrain_map> terrain;

    dirt_app(uvec2);
    void advance(SDL_Event);
    void draw();

    friend bool is_stopped(dirt_app const& self) { return self.quit; }

    void _init_controls();
    void _init_observers();

    auto insert_mesh(object_mesh mesh, Rxt::rgb color)
    {
        auto index = geom.insert(mesh);
        colors.emplace(index, color);
        return index;
    }
};
