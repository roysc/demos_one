#include "entity.hpp"
#include <Rxt/io.hpp>

namespace cpt
{
body build_plant()
{
    body::graph_type g;
    auto root = add_vertex(fvec3(0), g);
    auto top = add_vertex(fvec3(0,0,.25), g);
    add_edge(root, top, Rxt::colors::green, g);
    return {g};
}
}

void put_plant(entity_registry& r, ivec2 pos)
{
    using namespace cpt;
    auto e = r.create();

    r.emplace<zpos>(e, pos);
    r.emplace<body>(e, build_plant());
    r.emplace<life>(e);
    Rxt::print("creating plant at {}\n", pos);
}
