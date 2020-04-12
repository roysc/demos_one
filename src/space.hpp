#pragma once

#include <glm/glm.hpp>
#include <boost/multi_array.hpp>

#include <cmath>

using Vec4u8 = glm::tvec4<unsigned char, glm::highp>;
using image_data = boost::multi_array<Vec4u8, 2>;

// Clifford torus: the simplest and most symmetric flat embedding (in R^4)
// of the cartesian product of two circles
template <class FP, class Sampler>
FP sample_clifford_torus(FP nx, FP ny, const double R, Sampler&& sample_4D)
{
    using std::sin;
    using std::cos;
    const double TAU = 2 * M_PI;
    FP angle_x = TAU * nx, angle_y = TAU * ny;
    const auto C = R / TAU;

    return sample_4D(C * cos(angle_x), C * sin(angle_x),
                     C * cos(angle_y), C * sin(angle_y));
}

image_data create_map(glm::uvec2 size, int seed);
