#pragma once
#include "observable.hpp"

#include <glm/glm.hpp>

using ivec = glm::ivec2;
using uvec = glm::uvec2;

struct grid_viewport
{
    uvec max_scale; //grid_size
    uvec scale_factor {1};
    uvec size_px {max_scale * scale_factor};
    ivec _position {0};
    float margin_size = .1;

    observable<grid_viewport> _obs;
    auto hooks() { return _obs.hooks(); }
    friend void notify_observers(grid_viewport& v) { v._obs.notify_all(v); }
    // friend get_observer(grid_viewport& v) {return v._obs;}

    ivec position() const {return _position;}
    void position(ivec p) {_position = p; _obs.notify_all(*this); }

    void scale(int exp)
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
        _obs.notify_all(*this);
    }

    void move(int dx, int dy)
    {
        _position += ivec{dx, dy};
        _obs.notify_all(*this);
    }

    // size in number of cells
    uvec size_cells() const
    {
        return uvec(glm::vec2(size_px) / glm::vec2(scale_factor));
    }

    uvec size_pixels() const { return size_px; }

    bool edge_scroll(ivec cursor_position, int speed)
    {
        // (0,0) is center-screen, so offset it to the corner
        auto vpsize = size_cells();
        auto offset_pos = to_nds(cursor_position + ivec(vpsize / 2u));
        ivec dv {0};

        // std::cout << "edge_scroll? "
        //           << "nds=" << offset_pos
        for (unsigned i = 0; i < dv.length(); ++i) {
            if (offset_pos[i] < margin_size) {
                dv[i] = -speed;
                // dv[dv.offset_pos < margin_size] = -speed;
            } else if (offset_pos[i] + margin_size >= 2) {
                dv[i] = +speed;
            }
        }
        if (dv != ivec{0}) {
            move(dv.x, dv.y);
            return true;
        }
        return false;
    }

    auto from_nds(float x, float y) const
    {
        return floor(glm::vec2(x, y) * glm::vec2(size_cells() / 2u));
    }

    glm::vec2 to_nds(ivec p) const { return glm::vec2(p) / glm::vec2(size_cells() / 2u); }
};

template <class P>
struct _viewport;

template <>
struct _viewport<ivec> { using type = grid_viewport; };

template <class P>
using viewport = typename _viewport<P>::type;
