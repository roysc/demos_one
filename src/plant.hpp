#pragma once
#include "map.hpp"
#include "atrium.hpp"
#include "palette.hpp"
#include "entity.hpp"
#include "util.hpp"
#include "reactive.hpp"

#include "plant_models.hpp"

#include <string>
#include <vector>
#include <optional>
#include <map>
#include <cstddef>

using togopt_map = permissive_map<std::string, Rxt::reactive_toggle>;
namespace cpt = plant_cpt;

using geog_cell = std::uint8_t;
using geog_grid = dense_grid<geog_cell>;
using terrain_map = Rxt::adapt_reactive<geog_grid>;
// using geog_map = eulerian_space<geog_cell, 2>;
// using terrain_map = geog_map;

// map back to terrain grid for face selection
using face_to_space = std::map<mesh_data::source_face_descriptor, terrain_map::key_type>;

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
