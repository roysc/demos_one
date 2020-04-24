#pragma once
#include <glm/glm.hpp>

using ivec = glm::ivec2;
using uvec = glm::uvec2;

struct grid_view
{
    uvec max_scale; //grid_size
    uvec scale_factor {1};
    uvec viewport_size_px {max_scale * scale_factor};
    ivec viewport_position {0};

    void scale_viewport(int);

    void move_viewport(int dx, int dy)
    {
        viewport_position += ivec{dx, dy};
    }

    uvec viewport_size() const
    {
        return uvec(glm::vec2(viewport_size_px) / glm::vec2(scale_factor));
    }

    uvec viewport_size_pixels() const { return viewport_size_px; }
};
