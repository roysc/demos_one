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
