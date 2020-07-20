#pragma once
#include "map.hpp"
#include "geometry.hpp"
#include "index_mesh.hpp"
#include "util.hpp"
#include "reactive.hpp"

#include "atrium.hpp"
#include "palette.hpp"
#include "entity.hpp"

#include <string>
#include <vector>
#include <optional>
#include <map>
#include <cstddef>


using mesh3 = plaza::surface_mesh;
using mesh_data = plaza::indexed_mesh_vector<mesh3>;
using mesh_key = mesh_data::key_type;
using mesh_face = mesh_data::face_descriptor;
using mesh_colors = std::map<mesh_key, Rxt::rgba>;
// map to dependent faces
using foreign_face_map = std::map<mesh_face, mesh_face>;
using face_set = std::optional<mesh_face>;
using vertex_set = std::vector<mesh_data::vertex_descriptor>;

using uint8_grid = Rxt::adapt_reactive<dense_grid<std::uint8_t>>;
using terrain_map = uint8_grid;
// using heat_map = uint8_grid;

// map back to terrain grid for face selection
using face_to_space = std::map<mesh_data::source_face_descriptor, terrain_map::key_type>;

using toggle_map = permissive_map<std::string, Rxt::reactive_toggle>;

struct plant_app : atrium_app
{
    using super_type = atrium_app;

    color_palette palette;
    terrain_map terrain;
    entity_registry entities;
    entity_id e_debug;

    mesh_data geom;
    mesh_data ephem;
    mesh_colors colors, ephem_colors;
    foreign_face_map face_ephem;
    std::map<mesh_key, face_to_space> face_spaces;

    Rxt::adapt_reactive<face_set> highlighted_faces;
    Rxt::adapt_reactive<vertex_set> highlighted_vertices;

    toggle_map opts;

    Rxt::hooks<> _model_update, _ent_update;

    plant_app(uvec2);
    void _init_model();
    void _init_ui();
    void draw();

    Rxt::reactive_handle model_updates() override
    {
        return {
            &_model_update,
            &highlighted_faces.on_update,
        };
    }

    std::optional<ivec2> highlighted_space() const
    {
        if (highlighted_faces) {
            auto [oi, fd] = *highlighted_faces;
            ivec2 pos = face_spaces.at(oi).at(fd);
            assert(Rxt::point_within(pos, terrain.shape()));
            return pos;
        }
        return std::nullopt;
    }

    auto add_mesh(mesh3 mesh, Rxt::rgba color)
    {
        auto ix = geom.insert(mesh);
        geom.build();
        colors.emplace(ix, color);
        _model_update();
        return ix;
    }

    auto add_ephemeral(mesh3 mesh, Rxt::rgba color)
    {
        auto ix = ephem.insert(mesh);
        ephem.build();
        ephem_colors.emplace(ix, color);
        _model_update();
        return ix;
    }
};
