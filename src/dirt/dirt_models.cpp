#include "dirt_models.hpp"
#include "geometry_mesh.hpp"
#include <Rxt/geometry/shapes.hpp>

namespace dirt_ns
{
using Rxt::vec::fvec3;

skel_type build_plant(entity_registry)
{
    skel_type g;
    auto root = add_vertex(fvec3(0), g);
    return g;
}

skel_type build_kord()
{
    skel_type g;
    using namespace Rxt;
    rgb limb_clr = colors::hot_pink, head_clr = colors::red;

    // bones and main joints
    auto pelvis = add_vertex(fvec3(0, 0, .4), g);
    auto torso = add_vertex(fvec3(0, 0, .5), g);
    auto chest = add_vertex(fvec3(0, 0, .6), g);
    auto head = add_vertex(fvec3(0, 0, .7), g);

    add_edge(pelvis, torso, limb_clr, g);
    limb_clr = mix(limb_clr, head_clr, .5);
    add_edge(torso, chest, limb_clr, g);
    limb_clr = mix(limb_clr, head_clr, .5);
    add_edge(chest, head, limb_clr, g);

    // auto legs ;
    // auto arms ;

    return g;
}

skel_type build_man()
{
    using namespace atrium_geom;
    auto ret = build_kord();

    return ret;
}

mesh_type build_tetroid()
{
    using namespace atrium_geom;
    surface_mesh m;
    point bottom(0, 0, 0), front(0, .1, .8), left(-.1, -.4, .8), right(.1, -.4, .8);

    make_tetrahedron(bottom, front, left, right, m);
    return m;
}

mesh_type build_house()
{
    using namespace atrium_geom;
    surface_mesh g;
    point min{-.2, -.2, 0}, max{.2, .2, .1};
    auto hd = Rxt::make_cuboid(min, max, g);
    // delete floor & roof

    return g;
}

mesh_type build_wall() { return {}; }
}
