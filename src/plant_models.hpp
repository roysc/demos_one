#pragma once

#include "geometry.hpp"
#include "geometry_skel.hpp"
#include "geometry_mesh.hpp"
#include "entity.hpp"
#include "space.hpp"
#include "rendering.hpp"

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
    V offset() { return V(r, value()) + V(.5,.5,0); }
    // auto substage() {}
};

namespace planty
{
using mesh_type = plaza::surface_mesh;
using mesh_data = plaza::indexed_mesh_vector<mesh_type>;
using mesh_key = mesh_data::key_type;
using mesh_color = Rxt::rgba;
// using mesh_colors = std::map<mesh_key, mesh_color>;

using skel_type = plaza::skel_traits<Rxt::rgb, Rxt::fvec3>::graph_type;

// Could:
// - look up mesh from entity id
// -
struct mesh_geom
{
    using index_type = mesh_data;
    using key_type = mesh_key;
    using geometry_type = mesh_type;

    index_type* index;
    key_type key;
    mesh_color color;
    bool transparent = false;

    auto& get_geometry() { return index->get_source(key); }

    template <class B>
    void render(B& bufs, transform3 tm)
    {
        render_triangles(*this, bufs, tm);
    }
};

struct skel_geom
{
    skel_type _skel;

    auto& get_geometry() { return _skel; }

    template <class B>
    void render(B& bufs, transform3 tm)
    {
        render_skel(get_geometry(), bufs, tm);
    }
};

// using body_map = lagrangian_space<entity_id, float, 2>;
// using line_data = std::vector<std::pair<fvec3, fvec3>>;

namespace _cpt
{
using namespace plaza_ecs;
// struct input { input_event ; };

// struct zpos { Rxt::ivec2 r{0}; };
// struct zmove { enum { n, s, e, w } dir; };
// struct zvel { };
// struct fbox2 { Rxt::bounding_box<float> b; };
struct fpos3 { Rxt::fvec3 r{0}; };

using skel = skel_geom;
using mesh = mesh_geom;
using cell = stage_cell<zspace2::z2_stage>;
}

skel_type build_plant();
skel_type build_kord();
mesh_type build_tetroid();
mesh_type build_wall();
mesh_type build_house();
}
