#pragma once

#include "space.hpp"
#include "atrium/skeleton.hpp"

#include <entt/entt.hpp>

using entity_registry = entt::registry;

namespace cpt
{
struct zpos { ivec2 r; };
// struct fpos { fvec3 r; };
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

void put_body(entity_registry& r, ivec2 pos, skeleton);
