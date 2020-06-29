#include "controls.hpp"
#include "map.hpp"
#include "input.hpp"
#include "space.hpp"
#include "palette.hpp"

#include "entity.hpp"
#include "atrium/geometry.hpp"

#include <Rxt/graphics/shader/colored_triangle_3D.hpp>
#include <Rxt/graphics/shader/solid_color_3D.hpp>
#include <Rxt/graphics/sdl.hpp>
#include <Rxt/graphics/gl.hpp>
#include <Rxt/graphics/camera.hpp>
#include "reactive.hpp"

#include <map>
#include <optional>
#include <cstdint>

// using std::chrono::steady_clock;
// using time_point = steady_clock::time_point;

struct ui_traits
{
    using position_type = fvec2;
    using size_type = uvec2;
};

namespace sdl = Rxt::sdl;
using triangle_program = Rxt::shader_programs::colored_triangle_3D;
using line_program = Rxt::shader_programs::solid_color_3D<GL_LINES>;

using a3um::object_index;
using object_face = a3um::object_face_key;
using mesh_data = a3um::indexed_mesh_data;
using mesh_colors = std::map<object_index, Rxt::rgba>;

using Rxt::adapt_reactive_crt;
using Rxt::adapt_reactive;
using cursor_type = adapt_reactive_crt<reactive_cursor, Rxt::hooks<>, ui_traits>;
using camera_type = adapt_reactive_crt<reactive_focus_cam, Rxt::hooks<>>;
using hl_data = adapt_reactive<std::optional<object_face>>;
using terrain_map = adapt_reactive<dense_map<std::uint8_t>>;

// map back to terrain grid for face selection
using face_to_space = std::map<a3um::object_face_descriptor, terrain_map::key_type>;
// map to dependent faces
using foreign_face_map = std::map<object_face, object_face>;

struct dirt_app : public sdl::simple_gui
{
    bool quit = false;
    // time_point last_draw_time;
    input_hooks input;
    sdl::key_dispatcher keys;
    // sdl::metronome metronome;

    camera_type::position_type start_camera_at{8};
    camera_type camera{start_camera_at};
    cursor_type cursor;
    std::optional<ui_traits::position_type> drag_origin;

    triangle_program triangle_prog;
    triangle_program::data b_triangles {triangle_prog};
    triangle_program::data b_tris_txp {triangle_prog};
    line_program line_prog;
    line_program::data b_lines {line_prog};
    line_program::data b_uilines {line_prog};

    mesh_data geom;
    hl_data selected;
    mesh_data ephem;
    mesh_colors colors, ephem_colors;
    foreign_face_map face_ephem;
    std::map<object_index, face_to_space> face_spaces;

    color_palette palette;
    terrain_map terrain;
    entity_registry entreg;
    Rxt::hooks<> model_update, ent_update, on_debug;

    dirt_app(uvec2);
    void advance(SDL_Event);
    bool is_stopped() const { return quit; }
    void draw();

    void _init_controls();
    void _init_signals_ui();
    void _init_signals_model();

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

    std::optional<ivec2> selected_space() const;
};
