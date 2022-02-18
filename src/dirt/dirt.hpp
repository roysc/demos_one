#pragma once

#include "dirt_models.hpp"

#include "app3d.hpp"
#include "entity.hpp"
#include "palette.hpp"
#include "space.hpp"
#include "util.hpp"

#include <Rxt/reactive.hpp>
// #include <morton.h>

#include <map>
#include <optional>
#include <string>
#include <vector>

namespace cpt = dirt_ns::_cpt;
using Rxt::adapt_reactive;
// Toggleable options
using options_map = permissive_map<std::string, Rxt::reactive_toggle>;

using stage_type = zspace2::z2_stage;
using space_type = zspace2::z2_space;
using cell_position = stage_type::position_type;
using free_position = Rxt::vec::fvec3;

using mesh_index = dirt_ns::mesh_data;

// map to dependent faces
using mesh_face = mesh_index::face_descriptor;
using foreign_face_map = std::map<mesh_face, mesh_face>;
using face_set = std::optional<mesh_face>;
using vertex_set = std::vector<mesh_index::vertex_descriptor>;

// map back to terrain grid for face selection
using face_to_space = std::map<mesh_index::source_face_descriptor, cell_position>;
// indexed with mesh + face
using mesh_to_space = std::map<mesh_index::key_type, face_to_space>;

struct dirt_app : basic_app3d
{
    using super_type = basic_app3d;
    using position_type = cell_position;
    using mesh_type = mesh_index::source_mesh;
    using mesh_color = dirt_ns::mesh_color;
    using hook_type = Rxt::hooks<>;

    options_map opts;
    color_palette palette;

    hook_type _model_update, _ent_update;

    // geometry: 0 - spatially indexed, 1 - ephemeral
    mesh_index geom_, ephem_;
    mesh_index* _geom[2] = {&geom_, &ephem_};
    foreign_face_map face_ephem; // geom. faces to ephemeral dependencies
    mesh_to_space face_spaces;   // each mesh's faces -> grid spaces
    adapt_reactive<face_set> highlighted_faces;
    adapt_reactive<vertex_set> highlighted_vertices;

    space_type space;
    int _tick = 0;
    entity_registry entities;
    entity_id e_debug;
    Rxt::reactive_pointer<deep_stage<stage_type>> active_stage;

    dirt_app(viewport_size_type);
    void _init_model();
    void _init_ui();

    void advance(SDL_Event);
    void draw_clear();

    bool highlighted_space(position_type&) const;

    enum mesh_kind : bool
    {
        tangible = 0,
        ephemeral = 1
    };
    mesh_index& _mesh_index(mesh_kind mk = mesh_kind::tangible) { return *_geom[mk]; }
    entity_id put_mesh(mesh_type, mesh_color, bool, mesh_kind = mesh_kind::tangible);

    static auto default_camera(Rxt::vec::fvec2 map_size)
    {
        auto pos = free_position(map_size.x + map_size.y) / 2.f; // todo
        return super_type::camera_type(pos, free_position(Rxt::vec::fvec2(map_size) / 4.f, 0));
    }

    entity_id update_stage(stage_type&);
};
