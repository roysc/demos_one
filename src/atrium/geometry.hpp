#pragma once

#include <Rxt/geometry/mesh_transform.hpp>
#include <Rxt/geometry/triangle_primitive.hpp>

#include <CGAL/Simple_cartesian.h>
#include <CGAL/Surface_mesh.h>

#include <CGAL/AABB_tree.h>
#include <CGAL/AABB_traits.h>
#include <CGAL/Polygon_mesh_processing/triangulate_faces.h>

#include <boost/iterator/transform_iterator.hpp>
#include <boost/range/iterator.hpp>
#include <boost/range/iterator_range.hpp>

#include <map>
#include <vector>
#include <utility>

namespace atrium
{
namespace _g3d
{
using Kernel = CGAL::Simple_cartesian<float>;
using Point = CGAL::Point_3<Kernel>;
using Vector = CGAL::Vector_3<Kernel>;
using Ray = CGAL::Ray_3<Kernel>;

using Mesh = CGAL::Surface_mesh<Point>;
}

using object_mesh = _g3d::Mesh;
using triangle_mesh = _g3d::Mesh;

using mesh_transformer = Rxt::transform_comap_faces<object_mesh, triangle_mesh>;
using triangle_comap = mesh_transformer::face_comap;

using mesh_vector = std::vector<object_mesh>;
using object_index = mesh_vector::size_type;
using mesh_triangulations = std::vector<triangle_mesh>;
using triangle_comaps = std::map<object_index, triangle_comap>;

using object_face_descriptor = typename boost::graph_traits<object_mesh>::face_descriptor;
using object_face_key = std::pair<object_index, object_face_descriptor>;

using triangle_primitive = Rxt::triangle_primitive<mesh_vector>;
using triangle_aabb_tree = CGAL::AABB_tree<CGAL::AABB_traits<_g3d::Kernel, triangle_primitive>>;

// Struct for geometric mesh data with spatially indexed triangulations
// w/ faces mapped back to source mesh faces
struct mesh_data
// struct mesh_index
{
    // mesh_data& meshes;
    mesh_vector meshes;
    mesh_triangulations triangulations;
    triangle_comaps face_comaps;
    triangle_aabb_tree triangle_tree;

    object_index insert(object_mesh mesh)
    {
        object_index index = meshes.size();
        meshes.emplace_back(mesh);
        return index;
    }

    void build_triangulations()
    {
        mesh_transformer transformer{CGAL::Polygon_mesh_processing::triangulate_faces<triangle_mesh>};

        triangulations.clear();
        face_comaps.clear();

        unsigned index = 0;
        for (auto& mesh: meshes) {
            face_comaps.emplace(index, transformer(mesh, triangulations.emplace_back()));
            ++index;
        }
    }

    void index_triangles()
    {
        build_triangulations();
        for (unsigned ix = 0; ix < triangulations.size(); ++ix) {
            for (auto fd: faces(triangulations.at(ix))) {
                triangle_primitive prim{ix, fd};
                triangle_tree.insert(prim);
            }
        }
        triangle_tree.build(&triangulations);
    }
};
}
