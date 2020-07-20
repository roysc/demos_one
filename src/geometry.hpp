#pragma once

#include <CGAL/Simple_cartesian.h>
#include <CGAL/Surface_mesh.h>

namespace plaza_geom
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
using surface_mesh_vertex = surface_mesh::Vertex_index;
using surface_mesh_edge = surface_mesh::Edge_index;

template <class T>
point to_point(T);
}

namespace props
{
using CGAL::vertex_point;
}
