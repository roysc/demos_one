#include "grid_viewport.hpp"

void grid_viewport::scale(int exp)
{
    // simulate zoom in/out by scaling down/up resp.; correct position to keep centered
    const uvec min_scale{1};
    if (exp > 0) {
        if (scale_factor.x > min_scale.x && scale_factor.y > min_scale.y)
            scale_factor /= 2;
    } else {
        if (scale_factor.x < max_scale.x && scale_factor.y < max_scale.y)
            scale_factor *= 2;
    }
}
