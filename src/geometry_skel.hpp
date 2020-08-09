#pragma once
#include <Rxt/data/graph.hpp>

namespace plaza
{
// Wireframe object
template <class E, class Pos> //=Rxt::fvec3>
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
}
