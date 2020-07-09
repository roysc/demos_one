#pragma once

#include "spatial.hpp"
#include "skeleton.hpp"
#include "geometry.hpp"

#include <entt/entt.hpp>

using entity_registry = entt::registry;
using entity_id = entt::entity;

template <class C>
entity_id put_entity(entity_registry& r, ivec2 pos, C cpt);

////
struct species_registry
{
    // load from file?
};
struct species_id
{
    int index;
};

namespace cpt
{
struct zpos { ivec2 r; };
struct fpos { fvec3 r; };

using skel = skeleton;
using mesh = geometry::surface_mesh;

struct life
{
    species_id genes;
    int age = 0;
    void update() { age += 1; }
};
}

using mesh3 = geometry::surface_mesh;

skeleton build_plant();
skeleton build_kord();

mesh3 build_tetroid();
mesh3 build_house();
