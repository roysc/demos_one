#include "controls.hpp"
#include "reactive.hpp"
#include "map.hpp"
#include "input.hpp"
#include "entity.hpp"
#include "space.hpp"

#include "a3um/geometry.hpp"
#include "a3um/rendering.hpp"
#include "a3um/interaction.hpp"

#include <Rxt/graphics/shader/colored_triangle_3D.hpp>
#include <Rxt/graphics/shader/solid_color_3D.hpp>
#include <Rxt/graphics/sdl.hpp>
#include <Rxt/graphics/gl.hpp>
#include <Rxt/graphics/camera.hpp>
#include <Rxt/math.hpp>
#include <Rxt/time.hpp>
#include <Rxt/util.hpp>

#include <chrono>
#include <vector>

using std::chrono::steady_clock;
using time_point = steady_clock::time_point;

namespace gl = Rxt::gl;
namespace sdl = Rxt::sdl;
using triangle_program = Rxt::shader_programs::colored_triangle_3D;
using line_program = Rxt::shader_programs::solid_color_3D<GL_LINES>;

struct ui_traits
{
    using position_type = fvec2;
    using size_type = uvec2;
};

using namespace Rxt::frp;
using cursor_type = adapt_reactive_crt<reactive_cursor, hooks<>, ui_traits>;
using camera_type = adapt_reactive_crt<reactive_focus_cam, hooks<>>;

using mesh_data = a3um::indexed_mesh_data;
using a3um::mesh_colors;
using a3um::object_index;
using object_face = a3um::object_face_key;
using hl_data = adapt_reactive<a3um::hl_data>;

using int8 = unsigned char;
using terrain_value = int8;
using terrain_map = dense_map<terrain_value>;

// map back to terrain grid for face selection
using face_to_space = std::map<a3um::object_face_descriptor, terrain_map::key_type>;
// map to dependent faces
using foreign_face_map = std::map<object_face, object_face>;

struct dirt_app : public sdl::simple_gui
{
    sdl::key_dispatcher keys;
    input_hooks input;
    bool quit = false;
    sdl::metronome metronome {Rxt::duration_fps<30>(1), [this] { return !is_stopped(); }};
    // time_point last_draw_time;

    fvec3 start_camera_at{8};
    camera_type camera{start_camera_at};
    cursor_type cursor;

    triangle_program triangle_prog;
    triangle_program::data b_triangles {triangle_prog};
    triangle_program::data b_tris_alpha {triangle_prog};
    line_program line_prog;
    line_program::data b_lines {line_prog};
    line_program::data b_uilines {line_prog};

    mesh_data geom;
    hl_data hlite;
    mesh_data ephem;
    mesh_colors colors, ephem_colors;
    foreign_face_map face_ephem;
    std::map<object_index, face_to_space> face_spaces;

    adapt_reactive<terrain_map> terrain;
    entity_registry entreg;
    hooks<> model_update, ent_update, on_debug;

    dirt_app(uvec2);
    void advance(SDL_Event);
    bool is_stopped() const { return quit; }
    void draw();

    void _init_controls();
    void _init_observers();

    auto add_mesh(a3um::mesh mesh, Rxt::rgba color)
    {
        auto ix = geom.insert(mesh);
        geom.index_triangles();        
        colors.emplace(ix, color);
        model_update();
        return ix;
    }

    auto add_ephemeral(a3um::mesh mesh, Rxt::rgba color)
    {
        auto ix = ephem.insert(mesh);
        ephem.build_triangulations();
        ephem_colors.emplace(ix, color);
        model_update();
        return ix;
    }

    std::optional<ivec2> selected_space() const
    {
        if (hlite) {
            auto [oi, fd] = *hlite;
            ivec2 pos = face_spaces.at(oi).at(fd);
            assert(Rxt::point_within(pos, terrain.shape()));
            return pos;
        }
        return std::nullopt;
    }
};
