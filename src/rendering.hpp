#pragma once

#include "geometry.hpp"

#include <Rxt/geometry/helper.hpp>
#include <Rxt/graphics/color.hpp>
#include <Rxt/graphics/glm.hpp>

#include <CGAL/boost/graph/helpers.h>
#include <boost/property_map/property_map.hpp>

inline glm::vec3 to_glm(plaza_geom::point p) { return {p.x(), p.y(), p.z()}; }
inline glm::vec3 to_glm(plaza_geom::vector v) { return {v.x(), v.y(), v.z()}; }

template <class Trin, class Normals, class Color, class Bufs>
void render_mesh(Trin const& trin,
                 Normals get_normal,
                 Color color,
                 Bufs& bufs)
{
    for (auto fd: faces(trin)) {
        auto normal = get_normal(fd);
        for (auto point: Rxt::face_vertex_points<3>(trin, fd)) {
            auto p = to_glm(point);
            auto n = to_glm(normal);
            bufs.push(p, n, color);
        }
    }
}

template <class Index, class Colors, class Bufs>
void render_triangles(Index const& geom,
                      Colors const& colors,
                      Bufs& bufs)
{
    using TriMesh = typename Index::triangle_mesh;
    using TriFace = boost::graph_traits<TriMesh>::face_descriptor;
    using NormalMap = std::map<TriFace, plaza_geom::vector>;

    for (unsigned i = 0; i < geom.sources.size(); ++i) {
        auto& mesh = geom.sources.at(i);
        auto& trin = geom.triangulations.at(i);

        NormalMap normals;
        Rxt::calculate_face_normals(mesh, boost::make_assoc_property_map(normals));
        auto get_normal = [&, i] (auto fd) { return normals.at(geom.face_comaps.at(i).at(fd)); };

        render_mesh(trin, get_normal, colors.at(i), bufs);
    }
}

template <class Index, class LineBufs>
void render_hl(typename Index::face_descriptor fk,
               Index const& geom,
               LineBufs& lines,
               Rxt::rgb const color)
{
    auto [oi, fd] = fk;
    auto& mesh = geom.sources.at(oi);
    auto points = get(props::vertex_point, mesh);
    // Draw lines around face
    for (auto h: halfedges_around_face(halfedge(fd, mesh), mesh)) {
        lines.push(to_glm(points[source(h, mesh)]), color);
        lines.push(to_glm(points[target(h, mesh)]), color);
    }
}
