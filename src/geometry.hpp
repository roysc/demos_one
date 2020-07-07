#pragma once

#include <Rxt/geometry/mesh_transform.hpp>
#include <Rxt/geometry/triangle_primitive.hpp>
#include <Rxt/graphics/glm.hpp>

#include <CGAL/Simple_cartesian.h>
#include <CGAL/Surface_mesh.h>

#include <CGAL/AABB_tree.h>
#include <CGAL/AABB_traits.h>

#include <map>
#include <vector>
#include <utility>
#include <optional>

namespace a3um
{
namespace g3d
{
using Kernel = CGAL::Simple_cartesian<float>;
using Point = CGAL::Point_3<Kernel>;
using Vector = CGAL::Vector_3<Kernel>;
using Ray = CGAL::Ray_3<Kernel>;

using Mesh = CGAL::Surface_mesh<Point>;

using CGAL::vertex_point;
}
using kernel = g3d::Kernel;
using point = g3d::Point;
using vector = g3d::Vector;
using ray = g3d::Ray;
using mesh = g3d::Mesh;

inline g3d::Point to_point(glm::vec3 v) { return {v.x, v.y, v.z}; }
inline g3d::Point to_point(glm::vec2 v) { return {v.x, v.y, 0}; }

template <class Meshes, class Trins>
void build_triangulations(Meshes const&, Trins&);

template <class Meshes, class Trins, class Comaps>
void build_triangulations(Meshes const&, Trins&, Comaps&);

template <class Trins, class Index>
void index_triangles(Trins const&, Index&);

// Struct for geometric mesh data with spatially indexed triangulations
// w/ faces mapped back to source mesh faces
template <class Mesh>
struct indexed_mesh_vector
{
    using source_mesh = Mesh;
    using triangle_mesh = Mesh;
    using source_meshes = std::vector<Mesh>;
    using triangulated_meshes = std::vector<triangle_mesh>;
    using key_type = std::size_t;

    using source_face_descriptor = typename boost::graph_traits<source_mesh>::face_descriptor;
    using face_descriptor = std::pair<key_type, source_face_descriptor>;

    using mesh_transformer = Rxt::transform_comap_faces<source_mesh, triangle_mesh>;
    using triangle_comaps = std::map<key_type, typename mesh_transformer::face_comap>;
    using triangle_primitive = Rxt::triangle_primitive<triangulated_meshes>;
    using triangle_aabb_tree = CGAL::AABB_tree<CGAL::AABB_traits<g3d::Kernel, triangle_primitive>>;

    source_meshes sources;
    triangulated_meshes triangulations;
    triangle_comaps face_comaps;
    triangle_aabb_tree triangle_tree;

    auto insert(source_mesh mesh)
    {
        auto ix = sources.size();
        sources.emplace_back(mesh);
        return ix;
    }

    void build()
    {
        build_triangulations(sources, triangulations, face_comaps);
        index_triangles(triangulations, triangle_tree);
    }

    template <class Query>
    auto face_query(Query query) const
    {
        std::optional<face_descriptor> ret;

        if (auto opt = this->triangle_tree.first_intersected_primitive(query)) {
            auto [index, fd] = *opt;
            ret.emplace(index, this->face_comaps.at(index).at(fd));
        }
        return ret;
    }
};
}
