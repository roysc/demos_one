#pragma once

#include "spatial.hpp"
#include "geometry.hpp"
#include "geometry_skel.hpp"

#include <entt/entt.hpp>
#include <Rxt/demangle.hpp>

using entity_registry = entt::registry;
using entity_id = entt::entity;
inline constexpr entity_id nullent = entt::null;

template <class C>
entity_id put_entity(entity_registry&, ivec2, C);

std::string entity_name(entity_registry&, entity_id);

namespace _cpt
{
struct nam { std::string s; };
}
