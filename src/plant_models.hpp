#pragma once

#include "geometry.hpp"
#include "geometry_skel.hpp"
#include "geometry_mesh.hpp"
#include "entity.hpp"
#include "space.hpp"

#include <Rxt/color.hpp>

template <class S>
struct stage_cell
{
    // grid / float?
    // variant<ipos2, fpos3> r;
    using vec_type = typename S::position_type;
    S* env;
    vec_type r;

    stage_cell(S& e, vec_type v) : env(&e), r(v) {}

    auto value() { return env->grid().at(r); }
    template <class V>
    auto offset() { return V(r, value()) + V(.5,.5,0); }
    // auto substage() {}
};

template <class G>
struct geom3
{
    G geometry;

    auto& get_geometry() { return geometry; }
    template <class B>
    void render(B&, transform3);
};

namespace planty::_models
{
using mesh3 = plaza::surface_mesh;
// using skel3 = plaza::skel_graph;
using skel3 = plaza_geom::skel_traits<Rxt::rgb>::graph_type;
}

using mesh_type = planty::_models::mesh3;
using mesh_data = plaza::indexed_mesh_vector<mesh_type>;
using mesh_key = mesh_data::key_type;
using mesh_face = mesh_data::face_descriptor;
using mesh_color = Rxt::rgba;
// using mesh_colors = std::map<mesh_key, mesh_color>;
using skel_type = planty::_models::skel3;

// Could:
// - look up mesh from entity id
// -
template <>
struct geom3<mesh_type>
{
    using index_type = mesh_data;
    using key_type = mesh_key;

    index_type* index;
    key_type key;
    mesh_color color;
    bool transparent = false;

    auto& get_geometry() { return index->get(key); }

    template <class B>
    void render(geom3& g, B& bufs, transform3 tm);
    {
        auto& geom = g.get_geometry();
        render_triangles(geom, bufs, tm);
    }
};

template <class B>
void geom3<skel_type>::render(B& bufs, transform3 tm)
{
    render_skel(get_geometry(), bufs, tm);
}

// map to dependent faces
using foreign_face_map = std::map<mesh_face, mesh_face>;
using face_set = std::optional<mesh_face>;
using vertex_set = std::vector<mesh_data::vertex_descriptor>;

// using body_map = lagrangian_space<entity_id, float, 2>;
// using line_data = std::vector<std::pair<fvec3, fvec3>>;

namespace planty::_cpts
{
using namespace plaza_ecs;
// struct input { input_event ; };

// struct zpos { Rxt::ivec2 r{0}; };
// struct zmove { enum { n, s, e, w } dir; };
// struct zvel { };
// struct fbox2 { Rxt::bounding_box<float> b; };
struct fpos3 { Rxt::fvec3 r{0}; };

using skel = geom3<planty::_models::skel3>;
using mesh = geom3<mesh_type>;
using cell = stage_cell<z2_stage>;
}

namespace planty
{
using namespace _models;

skel3 build_plant();
skel3 build_kord();
mesh3 build_tetroid();
mesh3 build_wall();
mesh3 build_house();
}
