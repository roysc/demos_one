#pragma once

#include <entt/entt.hpp>
#include <Rxt/demangle.hpp>

using entity_registry = entt::registry;
using entity_id = entt::entity;
inline constexpr entity_id nullent = entt::null;

std::string entity_name(entity_registry&, entity_id);

namespace _cpt
{
template <class C> struct changing {};
struct nam { std::string s; };
}
