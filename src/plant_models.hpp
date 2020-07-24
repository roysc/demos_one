#pragma once

#include "geometry.hpp"
#include "geometry_skel.hpp"

namespace plant_model
{
using mesh3 = plaza::surface_mesh;
using skel3 = plaza::skel_graph;

skel3 build_plant();
skel3 build_kord();
mesh3 build_tetroid();
mesh3 build_wall();
mesh3 build_house();
}
