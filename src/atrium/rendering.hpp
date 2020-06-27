#pragma once

#include "geometry.hpp"
#include "interaction.hpp"

#include <Rxt/geometry/helper.hpp>
#include <Rxt/graphics/color.hpp>

#include <CGAL/boost/graph/helpers.h>
#include <boost/property_map/property_map.hpp>

#include <glm/glm.hpp>

namespace atrium
{
inline glm::vec3 to_glm(_g3d::Point p) { return {p.x(), p.y(), p.z()}; }
inline glm::vec3 to_glm(_g3d::Vector v) { return {v.x(), v.y(), v.z()}; }

using mesh_colors = std::map<object_index, Rxt::rgb>;

template <class Bufs>
void render_triangles(mesh_data const& geom,
                      mesh_colors const& colors,
                      Bufs& bufs)
{
    using Face = boost::graph_traits<triangle_mesh>::face_descriptor;
    using NormalMap = std::map<Face, _g3d::Vector>;

    for (object_index i = 0; i < geom.meshes.size(); ++i) {
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
void render_ux(object_face_key const& fk,
               mesh_data const& geom,
               LineBufs& lines)
{
    auto [oi, fd] = fk;
    auto& mesh = geom.meshes.at(oi);
    auto points = get(g3d::vertex_point, mesh);
    // Draw lines around face
    for (auto h: halfedges_around_face(halfedge(fd, mesh), mesh)) {
        lines.push(to_glm(points[source(h, mesh)]), Rxt::colors::white);
        lines.push(to_glm(points[target(h, mesh)]), Rxt::colors::white);
    }
}
}
