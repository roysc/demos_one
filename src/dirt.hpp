#include "controls.hpp"
#include "map.hpp"
#include "input.hpp"
#include "space.hpp"
#include "palette.hpp"

#include "entity.hpp"
#include "geometry.hpp"

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

using Rxt::adapt_reactive_crt;
using Rxt::adapt_reactive;
using cursor_type = adapt_reactive_crt<reactive_cursor, Rxt::hooks<>, ui_traits>;
using camera_state = Rxt::focused_camera;
using camera_type = adapt_reactive_crt<reactive_cam, Rxt::hooks<>, camera_state>;
using terrain_map = adapt_reactive<dense_map<std::uint8_t>>;

using mesh3 = geometry::surface_mesh;
using mesh_data = indexed_mesh_vector<mesh3>;
using mesh_key = mesh_data::key_type;
using mesh_face = mesh_data::face_descriptor;
using mesh_colors = std::map<mesh_key, Rxt::rgba>;
using optional_face = adapt_reactive<std::optional<mesh_face>>;

// map back to terrain grid for face selection
using face_to_space = std::map<mesh_data::source_face_descriptor, terrain_map::key_type>;
// map to dependent faces
using foreign_face_map = std::map<mesh_face, mesh_face>;

struct dirt_app : public sdl::simple_gui
{
    bool quit = false;
    // time_point last_draw_time;
    input_hooks input;
    sdl::key_dispatcher keys;
    // sdl::metronome metronome;

    uvec2 map_size;
    camera_state initial_camera;
    camera_type camera;
    cursor_type cursor;

    struct Drag {
        ui_traits::position_type pos;
        camera_state cam;
    };
    std::optional<Drag> drag_origin;

    color_palette palette;
    terrain_map terrain;
    entity_registry entities;
    entity_id e_debug;

    triangle_program triangle_prog;
    triangle_program::buffers b_triangles {triangle_prog};
    triangle_program::buffers b_tris_txp {triangle_prog};
    line_program line_prog, ui_line_prog;
    line_program::buffers b_lines {line_prog};
    line_program::buffers b_overlines {line_prog};
    line_program::buffers b_uilines {ui_line_prog};

    mesh_data geom;
    optional_face selected;
    mesh_data ephem;
    mesh_colors colors, ephem_colors;
    foreign_face_map face_ephem;
    std::map<mesh_key, face_to_space> face_spaces;

    Rxt::hooks<> model_update, ent_update, on_debug;

    dirt_app(uvec2);
    void advance(SDL_Event);
    bool is_stopped() const { return quit; }
    void draw();

    void _init_controls();
    void _init_signals_ui();
    void _init_signals_model();

    void handle_drag(fvec2, camera_state);
    std::optional<ivec2> selected_space() const;

    auto add_mesh(mesh3 mesh, Rxt::rgba color)
    {
        auto ix = geom.insert(mesh);
        geom.build();
        colors.emplace(ix, color);
        model_update();
        return ix;
    }

    auto add_ephemeral(mesh3 mesh, Rxt::rgba color)
    {
        auto ix = ephem.insert(mesh);
        ephem.build();
        ephem_colors.emplace(ix, color);
        model_update();
        return ix;
    }
};
