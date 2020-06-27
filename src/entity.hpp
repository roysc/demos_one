#pragma once

#include "space.hpp"
#include <Rxt/data/graph.hpp>
#include <Rxt/graphics/color.hpp>

#include <entt/entt.hpp>


using entity_registry = entt::registry;

namespace cpt
{
struct zpos { ivec2 r; };
struct fpos { fvec3 r; };
// struct vel { fvec3 dr; };

struct body
{
    struct vertex_t { using kind = Rxt::vertex_property_tag; };
    struct edge_t { using kind = Rxt::edge_property_tag; };
    using graph_type = Rxt::g_dl<Rxt::property<vertex_t, fvec3>,
                                 Rxt::property<edge_t, Rxt::rgb>>;
    graph_type graph;

    template <class Lines, class P>
    void render(Lines& lines, P offset)
    {
        auto pointpm = get(vertex_t{}, graph);
        auto edgepm = get(edge_t{}, graph);
        for (auto e: Rxt::to_range(edges(graph))) {
            auto color = edgepm[e];
            lines.push(pointpm[source(e, graph)] + offset , color);
            lines.push(pointpm[target(e, graph)] + offset, color);
        }
    }
};

struct life
{
    int age = 0;
    void update(body& bod) { age += 1; }
};
}

void put_plant(entity_registry& r, ivec2 pos);
