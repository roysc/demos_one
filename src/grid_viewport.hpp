#pragma once
#include <glm/glm.hpp>

using ivec = glm::ivec2;
using uvec = glm::uvec2;

struct grid_viewport
{
    uvec max_scale; //grid_size
    uvec scale_factor {4};
    uvec size_px {max_scale * scale_factor};
    ivec position {0};

    void scale(int);

    void move(int dx, int dy)
    {
        position += ivec{dx, dy};
    }

    // size in number of cells
    uvec size_cells() const
    {
        return uvec(glm::vec2(size_px) / glm::vec2(scale_factor));
    }

    uvec size_pixels() const { return size_px; }
};
