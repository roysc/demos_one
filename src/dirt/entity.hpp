#pragma once

#include <entt/entt.hpp>
#include <string>

using entity_registry = entt::registry;
using entity_id = entt::entity;
using entity_handle = entt::handle;

inline constexpr entity_id nullent = entt::null;

// Get entity name
std::string entity_name(entity_registry&, entity_id);

// Set (parent, child) relationship
// returns existing parent, or nullent
entity_id set_parent_entity(entity_registry&, entity_id, entity_id);
