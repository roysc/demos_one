#include "entity.hpp"

#include <Rxt/geometry/shapes.hpp>
#include <Rxt/io.hpp>

std::string entity_name(entity_registry& r, entity_id e)
{
    auto n = r.get<atrium_ecs::name>(e);
    return n.s;
}

// parent-child relationship
// return previous parent or nullent
entity_id set_parent_entity(entity_registry& r, entity_id par, entity_id child)
{
    namespace _cpt = atrium_ecs;
    auto& pc = r.get_or_emplace<_cpt::children>(par, _cpt::children{});
    pc.ids.insert(child);
    // just use two-way pointers
    entity_id ret = nullent;
    if (r.try_get<_cpt::parent>(child)) {
        ret = r.get<_cpt::parent>(child).id;
    }
    r.emplace<_cpt::parent>(child, par);
    return ret;
}
