#include "index_mesh.hpp"
#include "geometry.hpp"

#include <Rxt/geometry/ray_distance.hpp>

#include <CGAL/Search_traits_3.h>
#include <CGAL/Search_traits_adapter.h>
#include <CGAL/K_neighbor_search.h>
#include <CGAL/Orthogonal_k_neighbor_search.h>
#include <CGAL/property_map.h>

#include <utility>

namespace {
using namespace geometry;
using Mesh = surface_mesh;
using K = kernel;
using Search_point = std::pair<point, surface_mesh_vertex>;
using Search_traits = CGAL::Search_traits_adapter<
    Search_point,
    CGAL::First_of_pair_property_map<Search_point>,
    CGAL::Search_traits_3<K>>;
using Distance = Rxt::ray_distance<Search_point, K, ([] (auto& p) { return p.first; })>;
using Search = CGAL::K_neighbor_search<Search_traits, Distance>;
}

template <>
auto vertex_query(geometry::ray q, Search::Tree& ix, unsigned n)
{
    Distance dist;
    return Search(ix, q, n, 0, true, dist);
}
