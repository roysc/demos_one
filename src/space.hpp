#pragma once

#include <Rxt/math.hpp>

#include <glm/glm.hpp>
#include <boost/multi_array.hpp>

using Vec4u8 = glm::tvec4<unsigned char, glm::highp>;
using image_data = boost::multi_array<Vec4u8, 2>;

image_data create_map(glm::uvec2 size, int seed);
