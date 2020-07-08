#pragma once

#include <Rxt/graphics/glm.hpp>

using glm::ivec2;
using glm::uvec2;
using glm::fvec2;
using glm::fvec3;
using glm::fvec4;

template <class P, class S>
struct spatial_traits
{
    using position_type = P;
    using size_type = S;
};
