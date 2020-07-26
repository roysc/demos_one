#pragma once

#include "geometry.hpp"
#include "geometry_skel.hpp"
#include "geometry_mesh.hpp"
#include "entity.hpp"

namespace plant_model
{
using mesh3 = plaza::surface_mesh;
using skel3 = plaza::skel_graph;
}

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

// using body_map = lagrangian_space<entity_id, float, 2>;
// using line_data = std::vector<std::pair<fvec3, fvec3>>;

namespace plant_cpt
{
using namespace _cpt;
struct zpos { ivec2 r; };
struct fpos { fvec3 r; };
struct skel { plant_model::skel3 g; };
struct hitbox { Rxt::bounding_box<float> b; };

struct mesh
{
    using index_type = mesh_data;
    index_type* data;
    mesh_key key;
    mesh_color color;
    bool transparent = false;
};
}

namespace plant_model
{
skel3 build_plant();
skel3 build_kord();
mesh3 build_tetroid();
mesh3 build_wall();
mesh3 build_house();
}
