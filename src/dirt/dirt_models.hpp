#pragma once

#include "geometry.hpp"
#include "geometry_mesh.hpp"
#include "geometry_skel.hpp"
#include "rendering.hpp"
#include "space.hpp"

#include <Rxt/color.hpp>

using mesh_type = geom::surface_mesh;
using mesh_index = geom::indexed_mesh_vector<mesh_type>;
using mesh_key = mesh_index::key_type;
using mesh_color = Rxt::rgba;
// using mesh_colors = std::map<mesh_key, mesh_color>;

using skel_type = geom::skel_traits<Rxt::rgb, Rxt::vec::fvec3>::graph_type;

// using body_map = lagrangian_space<entity_id, float, 2>;
// using line_data = std::vector<std::pair<fvec3, fvec3>>;

struct fpos3
{
    Rxt::vec::fvec3 r;
};

using z2_traits = zspace2::spatial_traits;
using z2_cell = stage_cell<zspace2::z2_stage>;

// Could:
// - look up mesh from entity id
struct mesh_geom
{
    using index_type = mesh_index;
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

skel_type build_plant();
skel_type build_kord();

mesh_type build_tetroid();
mesh_type build_wall();
mesh_type build_house();
