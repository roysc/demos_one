#pragma once

#include "space.hpp"

#include <entt/entt.hpp>

struct agent_position
{
    grid_coord coord;
};

struct agent_traits
{
    bool male;
    unsigned char age;
};

auto create_agent(entt::registry& r, bool male, grid_coord pos)
{
    auto e = r.create();
    r.assign<agent_traits>(e, agent_traits {male, 0});
    r.assign<agent_position>(e, pos);
    return e;
}

auto displace(agent_position& pos, grid_offset d)
{
    pos.coord += d;
}
