#include "models.hpp"
#include "geometry_mesh.hpp"

skel build_plant()
{
    skel::graph_type g;
    auto root = add_vertex(fvec3(0), g);
    return skel(g);
}

skel build_kord()
{
    skel::graph_type g;
    using namespace Rxt;
    rgb limb_clr = colors::hot_pink,
        head_clr = colors::red;

    auto pelvis = add_vertex(fvec3(0,0,.4), g);
    auto torso = add_vertex(fvec3(0,0,.5), g);
    auto chest = add_vertex(fvec3(0,0,.6), g);
    auto head = add_vertex(fvec3(0,0,.7), g);

    add_edge(pelvis, torso, limb_clr, g);
    limb_clr = mix(limb_clr, head_clr, .5);
    add_edge(torso, chest, limb_clr, g);
    limb_clr = mix(limb_clr, head_clr, .5);
    add_edge(chest, head, limb_clr, g);

    // auto legs ;
    // auto arms ;

    return skel(g);
}

mesh3 build_tetroid()
{
    using namespace plaza_geom;
    surface_mesh m;
    make_tetrahedron(
        point(), point(), point(), point(),
        m);
    return m;
}

mesh3 build_house()
{
    using namespace plaza_geom;
    surface_mesh g;
    point min{-.4, -.4, 0}, max{.4, .4, .4};
    auto hd = Rxt::make_cuboid(min, max, g);
    // delete floor & roof

    return g;
}
