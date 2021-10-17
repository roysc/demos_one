#pragma once

#include <entt/entt.hpp>
#include <set>
#include <string>

using entity_registry = entt::registry;
using entity_id = entt::entity;
inline constexpr entity_id nullent = entt::null;

// Get entity name
std::string entity_name(entity_registry&, entity_id);

// Set (parent, child) relationship
// returns existing parent, or nullent
entity_id set_parent_entity(entity_registry&, entity_id, entity_id);

namespace atrium_ecs
{
// template <class C> struct changing {};

struct name { std::string s; };
struct parent { entity_id id; };
struct children { std::set<entity_id> ids; };
struct layer { int n; };

// namespace input {
// struct keyboard { };
// struct intelligence {};
// }

// struct mouse
// struct select {};
// struct highlight {};
}
