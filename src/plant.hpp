#pragma once

#include "plant_models.hpp"

#include "space.hpp"
#include "map.hpp"
#include "atrium.hpp"
#include "palette.hpp"
#include "entity.hpp"
#include "util.hpp"
#include "reactive.hpp"

// #include <morton.h>

#include <string>
#include <vector>
#include <functional>
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
    using universe_type = zspace2::z2_universe;
    using position_fvec = Rxt::fvec3;
    using position_ivec = zspace2::position_type;

    togopt_map opts;
    color_palette palette;

    universe_type universe;
    terrain_map terrain;

    // geographic_space geog;
    // std::vector<plant_stage> stage_cache;
    // plant_stage* active_stage;
    entity_registry entities;
    entity_id e_debug;

    mesh_data geom;             // spatially indexed geometry
    mesh_data ephem;            // ephemeral geometry
    foreign_face_map face_ephem; // geom. faces to ephemeral dependencies
    std::map<mesh_key, face_to_space> face_spaces; // each mesh's faces -> grid spaces
    Rxt::adapt_reactive<face_set> highlighted_faces;
    Rxt::adapt_reactive<vertex_set> highlighted_vertices;

    Rxt::hooks<> _model_update, _ent_update;

    plant_app(viewport_uvec);
    void _init_model();
    void _init_ui();

    void advance(SDL_Event);
    void draw_clear();

    bool highlighted_space(position_ivec&) const;
    mesh_key put_mesh(mesh_type, mesh_color, entity_id* = nullptr);
    mesh_key put_ephemeral(mesh_type, mesh_color, entity_id* = nullptr);

    static auto default_camera(Rxt::fvec2 map_size)
    {
        auto pos = position_fvec(map_size.x + map_size.y) / 2.f; // todo
        return super_type::camera_type(pos, position_fvec(Rxt::fvec2(map_size) / 4.f, 0));
    }
};
