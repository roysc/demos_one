#include "entity.hpp"
#include <Rxt/geometry/shapes.hpp>
#include <Rxt/io.hpp>

skeleton build_plant()
{
    skeleton::graph_type g;
    auto root = add_vertex(fvec3(0), g);
    auto top = add_vertex(fvec3(0,0,.25), g);
    add_edge(root, top, Rxt::colors::green, g);
    return skeleton(g);
}

skeleton build_kord()
{
    skeleton::graph_type g;
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

    return skeleton(g);
}

mesh3 build_tetroid()
{
    using namespace geometry;
    surface_mesh m;
    make_tetrahedron(
        point(), point(), point(), point(),
        m);
    return m;
}

mesh3 build_house()
{
    using namespace geometry;
    surface_mesh g;
    point min{-.4, -.4, 0}, max{.4, .4, .4};
    auto hd = Rxt::make_cuboid(min, max, g);
    // delete floor & roof

    return g;
}

template <>
entity_id put_entity(entity_registry& r, ivec2 pos, skeleton b)
{
    auto e = r.create();
    r.emplace<cpt::zpos>(e, pos);
    r.emplace<skeleton>(e, b);
    return e;
}

template <>
entity_id put_entity(entity_registry& r, ivec2 pos, cpt::mesh b)
{
    auto e = r.create();
    r.emplace<cpt::zpos>(e, pos);
    r.emplace<cpt::mesh>(e, b);
    return e;
}
