#include "geometry_mesh.hpp"
#include "geometry.hpp"

#include <CGAL/Polygon_mesh_processing/triangulate_faces.h>

namespace {
// Specialize for a vector of meshes
using Source = atrium_geom::surface_mesh;
using Trin = atrium_geom::surface_mesh;
using Sources = std::vector<Source>;
using Trins = std::vector<Trin>;
using Source_key = std::size_t;

using Mesh_transformer = Rxt::transform_comap_faces<Source, Trin>;
using Triangle_comaps = std::map<Source_key, typename Mesh_transformer::face_comap>;

using Triangle_primitive = Rxt::triangle_primitive<Trins>;
using Triangle_aabb_tree = CGAL::AABB_tree<CGAL::AABB_traits<atrium_geom::kernel, Triangle_primitive>>;
}

namespace atrium_geom
{
template <>
void build_triangulations(Sources const& meshes, Trins& triangulations)
{
    auto transform = [](auto const& src, auto& tgt)
    {
        CGAL::copy_face_graph(src, tgt);
        CGAL::Polygon_mesh_processing::triangulate_faces<Trin>(tgt);
    };
    triangulations.clear();
    for (auto& mesh: meshes) {
        transform(mesh, triangulations.emplace_back());
    }
}

template <>
void build_triangulations(Sources const& sources,
                          Trins& triangulations,
                          Triangle_comaps& face_comaps)
{
    Mesh_transformer transformer{CGAL::Polygon_mesh_processing::triangulate_faces<Trin>};

    unsigned i = 0;
    for (auto& mesh: sources) {
        face_comaps.emplace(i, transformer(mesh, triangulations.emplace_back()));
        ++i;
    }
}

template <>
void index_triangles(Trins const& triangulations, Triangle_aabb_tree& tree)
{
    for (unsigned ix = 0; ix < triangulations.size(); ++ix) {
        for (auto fd: faces(triangulations.at(ix))) {
            Triangle_primitive prim{ix, fd};
            tree.insert(prim);
        }
    }
    tree.build(&triangulations);
}
}
