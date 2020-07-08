#pragma once

#include "spatial.hpp"
#include "skeleton.hpp"

#include <entt/entt.hpp>

using entity_registry = entt::registry;
using entity_id = entt::entity;

namespace cpt
{
struct zpos { ivec2 r; };
struct fpos { fvec3 r; };
// struct vel { fvec3 dr; };
struct life
{
    int age = 0;
    void update() { age += 1; }
};
using skel = skeleton;

skeleton build_plant();
skeleton build_man();
}

entity_id put_body(entity_registry& r, ivec2 pos, skeleton);
