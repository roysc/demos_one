#pragma once

#include "space.hpp"
#include <Rxt/data/graph.hpp>
#include <Rxt/graphics/color.hpp>

#include <entt/entt.hpp>

struct skeleton
{
    struct vertex_t { using kind = Rxt::vertex_property_tag; };
    struct edge_t { using kind = Rxt::edge_property_tag; };
    using vert_prop = Rxt::property<vertex_t, fvec3>;
    using edge_prop = Rxt::property<edge_t, Rxt::rgb>;
    using graph_type = Rxt::g_dl<vert_prop, edge_prop>;
    graph_type graph;

    template <class Lines, class P>
    void render(Lines& lines, P offset)
    {
        auto vp = get(vertex_t{}, graph);
        auto ep = get(edge_t{}, graph);
        for (auto e: Rxt::to_range(edges(graph))) {
            auto color = ep[e];
            lines.push(vp[source(e, graph)] + offset , color);
            lines.push(vp[target(e, graph)] + offset, color);
        }
    }
};


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
