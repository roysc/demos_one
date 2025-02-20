#pragma once

#include "_debug.hpp"
#include "geometry.hpp"
#include "space.hpp"
// #include "geometry_skel.hpp"

#include <Rxt/color.hpp>
#include <Rxt/data/graph.hpp>
#include <Rxt/geometry/helper.hpp>
#include <Rxt/range.hpp>
#include <Rxt/vec.hpp>

#include <boost/property_map/property_map.hpp>

inline Rxt::vec::fvec3 to_glm(atrium_geom::point p) { return {p.x(), p.y(), p.z()}; }
inline Rxt::vec::fvec3 to_glm(atrium_geom::vector v) { return {v.x(), v.y(), v.z()}; }

template <class Trin, class Normals, class Color, class Bufs>
void render_mesh(Trin const& trin, Normals get_normal, Color color, Bufs& bufs, transform3 tmat)
{
    for (auto fd : faces(trin)) {
        auto normal = get_normal(fd);
        for (auto point : Rxt::face_vertex_points<3>(trin, fd)) {
            auto p = to_glm(point);
            auto n = to_glm(normal);

            p = apply(tmat, p);
            // print_debug("pushin' {}\n", p);
            bufs.push(p, n, color);
        }
    }
}

// todo - what is Mesh
template <class Mesh, class Bufs>
void render_triangles(Mesh& m, Bufs& bufs, transform3 tmat = Rxt::vec::fmat4(1))
{
    using Index = typename Mesh::index_type;
    using TriMesh = typename Index::triangle_mesh;
    using TriFace = typename Rxt::graph_traits<TriMesh>::face_descriptor;
    using NormalMap = std::map<TriFace, atrium_geom::vector>;

    auto i = m.key;
    Index& geom = *m.index;
    auto& mesh = geom.sources.at(i);
    auto& trin = geom.triangulations.at(i);

    NormalMap normals;
    Rxt::calculate_face_normals(mesh, boost::make_assoc_property_map(normals));
    auto get_normal = [&, i](auto fd) { return normals.at(geom.face_comaps.at(i).at(fd)); };

    render_mesh(trin, get_normal, m.color, bufs, tmat);
}

template <class Index, class LineBufs>
void render_hl(typename Index::face_descriptor fk, Index const& geom, LineBufs& lines,
               Rxt::rgb const color)
{
    auto [oi, fd] = fk;
    auto& mesh = geom.sources.at(oi);
    auto points = get(props::vertex_point, mesh);
    // Draw lines around face
    for (auto h : halfedges_around_face(halfedge(fd, mesh), mesh)) {
        lines.push(to_glm(points[source(h, mesh)]), color);
        lines.push(to_glm(points[target(h, mesh)]), color);
    }
}

// Can later use P as tmat for model or instance
template <class G, class Lines>
void render_skel(G const& g, Lines& lines, transform3 tm)
{
    using Tr = atrium::skel_traits<Rxt::rgb, Rxt::vec::fvec3>;
    // using skel_graph = color_graph_traits::graph_type;

    auto vp = get(Tr::vertex, g);
    auto ep = get(Tr::edge, g);
    for (auto e : Rxt::to_range(edges(g))) {
        auto color = ep[e];
        lines.push(apply(tm, vp[source(e, g)]), color);
        lines.push(apply(tm, vp[target(e, g)]), color);
    }
}
