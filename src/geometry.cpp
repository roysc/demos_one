#include "geometry.hpp"
#include <glm/glm.hpp>

namespace geom
{
template <>
point to_point(glm::vec3 v) { return {v.x, v.y, v.z}; }
template <>
point to_point(glm::vec2 v) { return {v.x, v.y, 0}; }
}
