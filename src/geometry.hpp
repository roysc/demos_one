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

namespace geometry
{
namespace cgal
{
using Kernel = CGAL::Simple_cartesian<float>;
using Point = CGAL::Point_3<Kernel>;
using Vector = CGAL::Vector_3<Kernel>;
using Ray = CGAL::Ray_3<Kernel>;
using Mesh = CGAL::Surface_mesh<Point>;
}
using kernel = cgal::Kernel;
using point = cgal::Point;
using vector = cgal::Vector;
using ray = cgal::Ray;
using surface_mesh = cgal::Mesh;

inline point to_point(glm::vec3 v) { return {v.x, v.y, v.z}; }
inline point to_point(glm::vec2 v) { return {v.x, v.y, 0}; }
}

namespace props
{
using CGAL::vertex_point;
}
