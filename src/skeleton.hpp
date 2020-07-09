#pragma once
#include <Rxt/graphics/color.hpp>
#include <Rxt/data/graph.hpp>
#include <Rxt/range.hpp>

struct skeleton
{
    struct vertex_t { using kind = Rxt::vertex_property_tag; };
    struct edge_t { using kind = Rxt::edge_property_tag; };
    using vert_prop = Rxt::property<vertex_t, fvec3>;
    using edge_prop = Rxt::property<edge_t, Rxt::rgb>;
    using graph_type = Rxt::directed_adj_list<vert_prop, edge_prop>;
    graph_type graph;
    skeleton(graph_type g) : graph(g) {}

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

