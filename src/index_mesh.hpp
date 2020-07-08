#pragma once

#include <Rxt/geometry/mesh_transform.hpp>
#include <Rxt/geometry/mesh_traits.hpp>
#include <Rxt/geometry/triangle_primitive.hpp>

#include <CGAL/AABB_tree.h>
#include <CGAL/AABB_traits.h>

#include <map>
#include <vector>
#include <utility>
#include <optional>

template <class Meshes, class Trins>
void build_triangulations(Meshes const&, Trins&);

template <class Meshes, class Trins, class Comaps>
void build_triangulations(Meshes const&, Trins&, Comaps&);

template <class Trins, class Index>
void index_triangles(Trins const&, Index&);

template <class Query, class Index>
auto vertex_query(Query, Index&, unsigned);

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

    using K = typename CGAL::Kernel_traits<typename Rxt::mesh_traits<triangle_mesh>::point>::Kernel;
    using mesh_transformer = Rxt::transform_comap_faces<source_mesh, triangle_mesh>;

    using triangle_comaps = std::map<key_type, typename mesh_transformer::face_comap>;
    using triangle_primitive = Rxt::triangle_primitive<triangulated_meshes>;
    using triangle_aabb_tree = CGAL::AABB_tree<CGAL::AABB_traits<K, triangle_primitive>>;

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
    friend auto face_query(Query query, indexed_mesh_vector const& ix)
    {
        std::optional<face_descriptor> ret;

        if (auto opt = ix.triangle_tree.first_intersected_primitive(query)) {
            auto [index, fd] = *opt;
            ret.emplace(index, ix.face_comaps.at(index).at(fd));
        }
        return ret;
    }
};
