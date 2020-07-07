#pragma once

#include <CGAL/Simple_cartesian.h>
#include <CGAL/Surface_mesh.h>

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

template <class T>
point to_point(T);
}

namespace props
{
using CGAL::vertex_point;
}
