#include "entity.hpp"
#include <Rxt/io.hpp>

namespace cpt
{
skeleton build_plant()
{
    skeleton::graph_type g;
    auto root = add_vertex(fvec3(0), g);
    auto top = add_vertex(fvec3(0,0,.25), g);
    add_edge(root, top, Rxt::colors::green, g);
    return {g};
}

skeleton build_man()
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

    return {g};
}
}

void put_body(entity_registry& r, ivec2 pos, cpt::skel b)
{
    using namespace cpt;
    auto e = r.create();

    r.emplace<zpos>(e, pos);
    r.emplace<skel>(e, b);
    // r.emplace<life>(e);
    Rxt::print("creating plant at {}\n", pos);
}
