#pragma once

#include <Rxt/geometry/mesh_traits.hpp>
#include <Rxt/geometry/mesh_transform.hpp>
#include <Rxt/geometry/triangle_primitive.hpp>
#include <Rxt/geometry/ray_distance.hpp>
#include <Rxt/util.hpp>

#include <CGAL/AABB_tree.h>
#include <CGAL/AABB_traits.h>

#include <CGAL/Search_traits_3.h>
#include <CGAL/Search_traits_adapter.h>
#include <CGAL/K_neighbor_search.h>
#include <CGAL/property_map.h>

#include <map>
#include <vector>
#include <utility>
#include <optional>

namespace atrium_geom
{
template <class Meshes, class Trins>
void build_triangulations(Meshes const&, Trins&);

template <class Meshes, class Trins, class Comaps>
void build_triangulations(Meshes const&, Trins&, Comaps&);

template <class Trins, class Index>
void index_triangles(Trins const&, Index&);

template <class M>
using mesh_kernel_t = typename CGAL::Kernel_traits<typename Rxt::mesh_traits<M>::point>::Kernel;

// Geometric mesh data with spatially indexed triangulations
// w/ faces mapped back to source mesh faces
template <class Mesh>
struct indexed_mesh_vector
{
    using self_type = indexed_mesh_vector;
    using key_type = std::size_t;
    using source_mesh = Mesh;
    using point_type = typename Rxt::mesh_traits<source_mesh>::point;

    using source_meshes = std::vector<source_mesh>;
    using source_traits = boost::graph_traits<source_mesh>;

    using source_face_descriptor = typename source_traits::face_descriptor;
    using face_descriptor = std::pair<key_type, source_face_descriptor>;
    using vertex_descriptor = std::pair<key_type, typename source_traits::vertex_descriptor>;

    // Ephemeral triangulation layer
    using triangle_mesh = Mesh;
    using triangulated_meshes = std::vector<triangle_mesh>;

    // Face indexing
    using mesh_transformer = Rxt::transform_comap_faces<source_mesh, triangle_mesh>;
    using triangle_comaps = std::map<key_type, typename mesh_transformer::face_comap>;
    using triangle_primitive = Rxt::triangle_primitive<triangulated_meshes>;
    using triangle_aabb_tree = CGAL::AABB_tree<
        CGAL::AABB_traits<mesh_kernel_t<triangle_mesh>, triangle_primitive>>;

    // Vertex indexing
    using triangle_point = typename Rxt::mesh_traits<triangle_mesh>::point;
    using ray_distance = Rxt::ray_distance<triangle_point, vertex_descriptor>;
    using ray_search_point = typename ray_distance::Point_d;
    using ray_search_traits = CGAL::Search_traits_adapter<
        ray_search_point,
        CGAL::First_of_pair_property_map<ray_search_point>,
        CGAL::Search_traits_3<mesh_kernel_t<source_mesh>>>;
    using ray_search = CGAL::K_neighbor_search<ray_search_traits, ray_distance>;
    using ray_search_tree = typename CGAL::K_neighbor_search<ray_search_traits, ray_distance>::Tree;

    // Edge indexing :todo

    source_meshes sources;
    triangulated_meshes triangulations;
    triangle_comaps face_comaps;
    triangle_aabb_tree triangle_tree;
    ray_search_tree point_tree;

    auto insert(source_mesh mesh)
    {
        auto ix = sources.size();
        sources.emplace_back(mesh);
        for (auto v: vertices(mesh)) {
            point_tree.insert({get(CGAL::vertex_point, mesh, v), {ix, v}});
        }
        return ix;
    }

    auto& get_source(key_type k)
    {
        return sources.at(k);
    }

    // auto& get(key_type k) const
    // {
    //     return get_source(k);
    // }

    void build()
    {
        triangulations.clear();
        face_comaps.clear();
        build_triangulations(sources, triangulations, face_comaps);
        // if constexpr (no_index) return;
        triangle_tree.clear();
        index_triangles(triangulations, triangle_tree);
    }

    void clear() {}

    template <class Q>
    friend auto face_query(Q query, self_type const& ix)
    {
        // static_assert(!no_index);
        std::optional<face_descriptor> ret;
        if (auto opt = ix.triangle_tree.first_intersected_primitive(query)) {
            auto [index, fd] = *opt;
            ret.emplace(index, ix.face_comaps.at(index).at(fd));
        }
        return ret;
    }

    template <class Q>
    friend auto vertex_query(Q q, self_type const& ix, unsigned n)
    {
        // static_assert(!no_index);
        ray_distance dist;
        return ray_search(ix.point_tree, q, n, 0, true, dist);
    }
};
}
