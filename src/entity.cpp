#include "entity.hpp"
#include "matter.hpp"

#include <Rxt/geometry/shapes.hpp>
#include <Rxt/io.hpp>

std::string entity_name(entity_registry& r, entity_id e)
{
    auto n = r.get<_cpt::nam>(e);
    return n.s;
}
