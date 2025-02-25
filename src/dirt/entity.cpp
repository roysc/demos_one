#include "entity.hpp"

#include <Rxt/geometry/shapes.hpp>
#include <Rxt/io.hpp>

namespace ecs
{
struct name { std::string s; };
struct parent { entity_id id; };
struct children { std::set<entity_id> ids; };
struct layer { int n; };
}

std::string entity_name(entity_registry& r, entity_id e)
{
    auto n = r.get<ecs::name>(e);
    return n.s;
}

// parent-child relationship
// return previous parent or nullent
entity_id set_parent_entity(entity_registry& r, entity_id par, entity_id child)
{
    auto& pc = r.get_or_emplace<ecs::children>(par, ecs::children{});
    pc.ids.insert(child);
    // just use two-way pointers
    entity_id ret = nullent;
    if (r.try_get<ecs::parent>(child)) {
        ret = r.get<ecs::parent>(child).id;
    }
    r.emplace<ecs::parent>(child, par);
    return ret;
}
