#include "geometry.hpp"

#include <CGAL/Polygon_mesh_processing/triangulate_faces.h>

namespace a3um
{
void basic_mesh_data::build_triangulations()
{
    auto transform = [](auto const& src, auto& tgt)
    {
        CGAL::copy_face_graph(src, tgt);
        CGAL::Polygon_mesh_processing::triangulate_faces<triangle_mesh>(tgt);
    };
    triangulations.clear();
    for (auto& mesh: meshes) {
        transform(mesh, triangulations.emplace_back());
    }
}

void indexed_mesh_data::build_triangulations()
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

void indexed_mesh_data::index_triangles()
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
}
