#pragma once

#include "spatial.hpp"
#include "geometry.hpp"

#include "skeleton.hpp"

#include <entt/entt.hpp>
#include <Rxt/demangle.hpp>

using entity_registry = entt::registry;
using entity_id = entt::entity;

template <class C>
entity_id put_entity(entity_registry&, ivec2, C);

std::string entity_name(entity_registry&, entity_id);

namespace _cpt
{
struct zpos { ivec2 r; };
struct fpos { fvec3 r; };
struct nam { std::string s; };

using skel = plaza_geom::skeleton<Rxt::rgb>;
using mesh = plaza_geom::surface_mesh;
}

using mesh3 = _cpt::mesh;
using _cpt::skel;

skel build_plant();
skel build_kord();

mesh3 build_tetroid();

mesh3 build_wall();
mesh3 build_house();

template <class C>
entity_id put_entity(entity_registry& r, ivec2 pos, C cpt)
{
    auto e = r.create();
    r.emplace<_cpt::zpos>(e, pos);
    r.emplace<C>(e, cpt);
    r.emplace<_cpt::nam>(e, Rxt::type_name<C>());
    return e;
}
