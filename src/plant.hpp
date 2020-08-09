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
namespace cpt = planty::_cpts;
using Rxt::adapt_reactive;

using geog_cell = std::uint8_t;
using geog_grid = dense_grid<geog_cell>;
using terrain_map = adapt_reactive<geog_grid>;

// map back to terrain grid for face selection
using face_to_space = std::map<mesh_data::source_face_descriptor, terrain_map::key_type>;

struct plant_app : atrium_app
{
    using super_type = atrium_app;

    using stage_type = zspace2::z2_stage;
    using universe_type = zspace2::z2_universe;
    using position_ivec = stage_type::position_type;
    using position_fvec = Rxt::fvec3;

    togopt_map opts;
    color_palette palette;

    universe_type universe;
    Rxt::reactive_pointer<stage_type> active_stage;
    entity_registry entities;
    entity_id e_debug;

    // geometry: 0 - spatially indexed, 1 - ephemeral
    mesh_data geom;
    mesh_data ephem;
    mesh_data* _geom[2] = {&geom, &ephem};
    foreign_face_map face_ephem; // geom. faces to ephemeral dependencies
    std::map<mesh_key, face_to_space> face_spaces; // each mesh's faces -> grid spaces
    adapt_reactive<face_set> highlighted_faces;
    adapt_reactive<vertex_set> highlighted_vertices;

    Rxt::hooks<> _model_update, _ent_update;

    plant_app(viewport_uvec);
    void _init_model();
    void _init_ui();

    void advance(SDL_Event);
    void draw_clear();

    bool highlighted_space(position_ivec&) const;
    entity_id put_mesh(mesh_type, mesh_color, bool = false);

    static auto default_camera(Rxt::fvec2 map_size)
    {
        auto pos = position_fvec(map_size.x + map_size.y) / 2.f; // todo
        return super_type::camera_type(pos, position_fvec(Rxt::fvec2(map_size) / 4.f, 0));
    }

    void load_stage(stage_type&);
};
