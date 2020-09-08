#pragma once

#include "geometry.hpp"
#include "geometry_skel.hpp"
#include "geometry_mesh.hpp"
#include "entity.hpp"
#include "space.hpp"
#include "rendering.hpp"

#include <Rxt/color.hpp>
#include <optional>

using tick = int;
using tick_delta = int;

struct alive
{
    tick born_at;
};

// skeletal_animation


namespace planty
{
using mesh_type = atrium::surface_mesh;
using mesh_data = atrium::indexed_mesh_vector<mesh_type>;
using mesh_key = mesh_data::key_type;
using mesh_color = Rxt::rgba;
// using mesh_colors = std::map<mesh_key, mesh_color>;

using skel_type = atrium::skel_traits<Rxt::rgb, Rxt::fvec3>::graph_type;

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

using z2_traits = zspace2::spatial_traits;
namespace _phys
{
struct motion { z2_traits::velocity v; };

}

namespace _cpt
{
using namespace atrium_ecs;
using namespace _phys;

// (asynchronous?) input component
// -prompt
// -generate
// -rpc?
template <class C>
struct input
{};

// struct zpos { Rxt::ivec2 r{0}; };
// struct zvel { };
// struct fbox2 { Rxt::bounding_box<float> b; };
// struct fpos3 { Rxt::fvec3 r{0}; };
struct fpos3 { Rxt::fvec3 r; };

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
