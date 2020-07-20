#include "controls.hpp"
#include "map.hpp"
#include "input.hpp"
#include "spatial.hpp"
#include "palette.hpp"
#include "buffer_index.hpp"
#include "entity.hpp"
#include "geometry.hpp"
#include "index_mesh.hpp"
#include "viewport.hpp"
#include "util.hpp"

#include <Rxt/graphics/shader/colored_triangle_3D.hpp>
#include <Rxt/graphics/shader/solid_color_3D.hpp>
#include <Rxt/graphics/sdl.hpp>
#include <Rxt/graphics/camera.hpp>
#include "reactive.hpp"

#include <map>
#include <optional>
#include <cstdint>

namespace plaza
{
using namespace plaza_geom;
}

// using std::chrono::steady_clock;
// using time_point = steady_clock::time_point;
namespace sdl = Rxt::sdl;
using Rxt::adapt_reactive_crt;
using Rxt::adapt_reactive;

using triangle_program = Rxt::shader_programs::colored_triangle_3D;
using line_program = Rxt::shader_programs::solid_color_3D<GL_LINES>;
using point_program = Rxt::shader_programs::solid_color_3D<GL_POINTS>;

struct cursor_traits
{
    using position_type = fvec2;
    // using size_type = uvec2;
};

using cursor_type = adapt_reactive_crt<reactive_cursor, Rxt::hooks<>, cursor_traits>;

using camera_state = Rxt::focused_camera;
using camera_type = adapt_reactive_crt<reactive_cam, Rxt::hooks<>, camera_state>;
struct drag_state { cursor_traits::position_type pos; camera_state cam; };

using panel_traits = spatial_traits<ivec, uvec>;
using panel_viewport = basic_viewport<panel_traits>;
using panel_layer = std::vector<std::pair<ivec, ivec>>; // todo index

using mesh3 = plaza::surface_mesh;
using mesh_data = plaza::indexed_mesh_vector<mesh3>;
using mesh_key = mesh_data::key_type;
using mesh_face = mesh_data::face_descriptor;
using mesh_colors = std::map<mesh_key, Rxt::rgba>;
// map to dependent faces
using foreign_face_map = std::map<mesh_face, mesh_face>;
using face_set = std::optional<mesh_face>;
using vertex_set = std::vector<mesh_data::vertex_descriptor>;

using uint8_grid = adapt_reactive<dense_grid<std::uint8_t>>;
using terrain_map = uint8_grid;
// using heat_map = uint8_grid;

// map back to terrain grid for face selection
using face_to_space = std::map<mesh_data::source_face_descriptor, terrain_map::key_type>;

struct reactive_toggle : Rxt::toggle_hooks
{
    bool state = false;
    reactive_toggle() {}
    operator bool() const { return state; }

    void disable()
    {
        state = false;
        this->on_disable();
    }

    void enable()
    {
        state = true;
        this->on_enable();
    }
};

template <class K, class V>
struct grab_map
{
    using key_type = K;
    using value_type = V;
    std::map<K, V> _map;

    value_type& operator[](key_type k)
    {
        return get_or_emplace(_map, k, V{});
    }
};

using toggle_map = grab_map<std::string, reactive_toggle>;

template <class T>
struct view_mode
{
    view_mode() {}
};

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
    std::optional<drag_state> drag_origin;

    panel_viewport ui_viewport;
    panel_layer ui_objects;

    color_palette palette;
    terrain_map terrain;
    entity_registry entities;
    entity_id e_debug;

    triangle_program triangle_prog;
    line_program line_prog, ui_line_prog;
    point_program point_prog;
    buffer_index buf;

    mesh_data geom;
    mesh_data ephem;
    mesh_colors colors, ephem_colors;
    foreign_face_map face_ephem;
    std::map<mesh_key, face_to_space> face_spaces;

    adapt_reactive<face_set> highlighted_faces;
    adapt_reactive<vertex_set> highlighted_vertices;

    toggle_map opts;
    Rxt::hooks<> model_update, ent_update, on_debug;

    dirt_app(uvec2);
    void advance(SDL_Event);
    bool is_stopped() const { return quit; }
    void draw();

    void _init_controls();
    void _init_signals_ui();
    void _init_signals_model();

    void handle_drag(fvec2, camera_state);
    std::optional<ivec2> highlighted_space() const;

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
