#pragma once

#include "geometry.hpp"

#include <Rxt/geometry/helper.hpp>
#include <Rxt/graphics/color.hpp>
#include <Rxt/graphics/glm.hpp>

#include <CGAL/boost/graph/helpers.h>
#include <boost/property_map/property_map.hpp>

namespace a3um
{
inline glm::vec3 to_glm(g3d::Point p) { return {p.x(), p.y(), p.z()}; }
inline glm::vec3 to_glm(g3d::Vector v) { return {v.x(), v.y(), v.z()}; }

template <class Colors, class Bufs>
void render_triangles(indexed_mesh_data const& geom,
                      Colors const& colors,
                      Bufs& bufs)
{
    using TriFace = boost::graph_traits<indexed_mesh_data::triangle_mesh>::face_descriptor;
    using NormalMap = std::map<TriFace, g3d::Vector>;

    for (unsigned i = 0; i < geom.meshes.size(); ++i) {
        auto& mesh = geom.meshes.at(i);
        auto& t3n = geom.triangulations.at(i);

        NormalMap normals;
        Rxt::calculate_face_normals(mesh, boost::make_assoc_property_map(normals));

        auto color = colors.at(i);

        for (auto fd: faces(t3n)) {
            auto normal = normals.at(geom.face_comaps.at(i).at(fd));
            for (auto point: Rxt::face_vertex_points<3>(t3n, fd)) {
                auto p = to_glm(point);
                auto n = to_glm(normal);
                bufs.push(p, n, color);
            }
        }
    }
}

template <class LineBufs>
void render_hl(basic_mesh_data::object_face_key const& fk,
               basic_mesh_data const& geom,
               LineBufs& lines,
               Rxt::rgb const color)
{
    auto [oi, fd] = fk;
    auto& mesh = geom.meshes.at(oi);
    auto points = get(g3d::vertex_point, mesh);
    // Draw lines around face
    for (auto h: halfedges_around_face(halfedge(fd, mesh), mesh)) {
        lines.push(to_glm(points[source(h, mesh)]), color);
        lines.push(to_glm(points[target(h, mesh)]), color);
    }
}
}
