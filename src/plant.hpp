#pragma once
#include "map.hpp"
#include "geometry.hpp"
#include "geometry_mesh.hpp"
#include "util.hpp"
#include "reactive.hpp"
#include "atrium.hpp"
#include "palette.hpp"
#include "entity.hpp"

#include "plant_models.hpp"

#include <string>
#include <vector>
#include <optional>
#include <map>
#include <cstddef>


using mesh_type = plant_model::mesh3;
using mesh_data = plaza::indexed_mesh_vector<mesh_type>;
using mesh_key = mesh_data::key_type;
using mesh_face = mesh_data::face_descriptor;
using mesh_color = Rxt::rgba;
// using mesh_colors = std::map<mesh_key, mesh_color>;

// map to dependent faces
using foreign_face_map = std::map<mesh_face, mesh_face>;
using face_set = std::optional<mesh_face>;
using vertex_set = std::vector<mesh_data::vertex_descriptor>;

using geog_cell = std::uint8_t;
using geog_grid = dense_grid<geog_cell>;
using terrain_map = Rxt::adapt_reactive<geog_grid>;
// using geog_map = eulerian_space<geog_cell, 2>;
// using terrain_map = geog_map;

// using body_map = lagrangian_space<entity_id, float, 2>;

// map back to terrain grid for face selection
using face_to_space = std::map<mesh_data::source_face_descriptor, terrain_map::key_type>;

using togopt_map = permissive_map<std::string, Rxt::reactive_toggle>;
using line_data = std::vector<std::pair<fvec3, fvec3>>;

namespace plant_cpt
{
using namespace _cpt;
struct zpos { ivec2 r; };
struct fpos { fvec3 r; };
struct skel { plant_model::skel3 g; };
struct mesh
{
    using index_type = mesh_data;
    index_type* data;
    mesh_key key;
    mesh_color color;
    bool transparent = false;
};
}
namespace cpt = plant_cpt;

struct plant_app : atrium_app
{
    using super_type = atrium_app;

    togopt_map opts;
    Rxt::hooks<> _model_update, _ent_update;

    color_palette palette;
    terrain_map terrain;
    // geographic_space geog;

    entity_registry entities;
    entity_id e_debug;

    mesh_data geom;             // spatially indexed geometry
    line_data lines;
    mesh_data ephem;            // ephemeral geometry
    // mesh_colors colors, ephem_colors;
    foreign_face_map face_ephem; // geom. faces to ephemeral dependencies
    std::map<mesh_key, face_to_space> face_spaces; // each mesh's faces -> grid spaces

    Rxt::adapt_reactive<face_set> highlighted_faces;
    Rxt::adapt_reactive<vertex_set> highlighted_vertices;

    plant_app(uvec2);
    void _init_model();
    void _init_ui();

    void advance(SDL_Event);
    void draw_clear();

    std::optional<ivec2> highlighted_space() const;
    mesh_key put_mesh(mesh_type, mesh_color, entity_id=nullent);
    mesh_key put_ephemeral(mesh_type, mesh_color, entity_id=nullent);
};
