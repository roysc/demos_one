#pragma once
#include <Rxt/graphics/color.hpp>
#include <Rxt/data/graph.hpp>
#include <Rxt/range.hpp>

namespace plaza_geom
{
// template <class J, class B>
// using skel_graph = Rxt::directed_adj_list<J, B>;
template <class T>
struct skeleton
{
    struct vertex_t { using kind = Rxt::vertex_property_tag; };
    struct edge_t { using kind = Rxt::edge_property_tag; };
    static constexpr vertex_t vertex{};
    static constexpr edge_t edge{};

    using vert_prop = Rxt::property<vertex_t, fvec3>;
    using edge_prop = Rxt::property<edge_t, T>;
    using graph_type = Rxt::directed_adj_list<vert_prop, edge_prop>;

    using vd = Rxt::graph_traits<graph_type>::vertex_descriptor;
    using ed = Rxt::graph_traits<graph_type>::edge_descriptor;

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
}
