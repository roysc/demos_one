#pragma once
#include <Rxt/graphics/color.hpp>
#include <Rxt/graphics/glm.hpp>
#include <Rxt/data/graph.hpp>
#include <Rxt/range.hpp>

namespace plaza_geom
{
// Wireframe object
template <class E, class Pos=glm::fvec3>
struct skel_traits
{
    struct vertex_t { using kind = Rxt::vertex_property_tag; };
    struct edge_t { using kind = Rxt::edge_property_tag; };
    static constexpr vertex_t vertex{};
    static constexpr edge_t edge{};

    // dual-quat for rigged skel?
    using vert_prop = Rxt::property<vertex_t, Pos>;
    using edge_prop = Rxt::property<edge_t, E>;
    using graph_type = Rxt::directed_adj_list<vert_prop, edge_prop>;

    using vdesc = Rxt::graph_traits<graph_type>::vertex_descriptor;
    using edesc = Rxt::graph_traits<graph_type>::edge_descriptor;
};

using color_graph_traits = skel_traits<Rxt::rgb>;
using skel_graph = color_graph_traits::graph_type;

// Can later use P as tmat for model or instance
template <class Lines, class P>
void render_skel(skel_graph const& g, Lines& lines, P offset)
{
    using Tr = color_graph_traits;
    auto vp = get(Tr::vertex, g);
    auto ep = get(Tr::edge, g);
    for (auto e: Rxt::to_range(edges(g))) {
        auto color = ep[e];
        lines.push(vp[source(e, g)] + offset , color);
        lines.push(vp[target(e, g)] + offset, color);
    }
}
}
