#pragma once

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/transform.hpp>
#include <glm/glm.hpp>
#include <type_traits>

using ivec = glm::ivec2;
using uvec = glm::uvec2;

template <class ST>
struct old_viewport
{
    using traits_type = ST;
    using position_type = typename ST::position_type;
    using size_type = typename ST::size_type;

    using P = position_type;
    using Size = size_type;

    Size max_scale; //grid_size
    Size scale_factor {1};
    P _position {0};
    const Size size_px {max_scale * scale_factor};
    const float margin_size = .1;

    old_viewport(Size max, Size scale = Size(1))
        : max_scale{max}, scale_factor{scale}
    {
        auto gt_zero = [](auto p) {
            Size zero{0};
            return (p[0] > zero[0] && p[1] > zero[1]);
        };
        assert(gt_zero(scale));
        assert(gt_zero(max));
    }

    P position() const { return _position; }

    virtual void scale(int exp)
    {
        // simulate zoom in/out by scaling down/up resp.; correct position to keep centered
        const Size min_scale{1};
        if (exp > 0) {
            if (scale_factor.x > min_scale.x && scale_factor.y > min_scale.y)
                set_scale(scale_factor / 2);
        } else {
            if (scale_factor.x < max_scale.x && scale_factor.y < max_scale.y)
                set_scale(scale_factor * 2);
        }
    }

    virtual void move(P d)
    {
        _position += d;
    }

    // size in number of cells
    Size size_cells() const
    {
        return Size(glm::vec2(size_pixels()) / glm::vec2(scale_factor));
    }

    Size size_pixels() const { return size_px; }

    bool edge_scroll(P cursor_position, int speed)
    {
        // (0,0) is center-screen, so offset it to the corner
        auto vpsize = size_cells();
        auto offset_pos = to_nds(cursor_position + P(vpsize / 2u));
        P dv {0};

        for (unsigned i = 0; i < dv.length(); ++i) {
            if (offset_pos[i] < margin_size) {
                dv[i] = -speed;
                // dv[dv.offset_pos < margin_size] = -speed;
            } else if (offset_pos[i] + margin_size >= 2) {
                dv[i] = +speed;
            }
        }
        if (dv != P{0}) {
            move(dv);
            return true;
        }
        return false;
    }

    auto from_nds(float x, float y) const
    {
        return floor(glm::vec2(x, y) * glm::vec2(size_cells() / 2u));
    }

    glm::vec2 to_nds(P p) const { return glm::vec2(p) / glm::vec2(size_cells() / 2u); }

    glm::mat4 view_matrix() const
    {
        using glm::vec2;
        using glm::vec3;

        auto pos3 = vec3(position(), 0);
        auto cells = vec3(size_cells(), 1);
        glm::mat4 view_matrix =
            glm::scale(vec3(2.f / vec2(cells), 0)) *
            glm::translate(-pos3);
        return view_matrix;
    }

    glm::mat4 model_matrix() const
    {
        // return glm::translate(glm::vec3(.5, .5, 0)); // offset to center of cell
        return glm::mat4{1};
    }

    template <class P>
    void update_uniforms(P& p, bool pos = true)
    {
        set(p->viewport_size, size_cells());
        if (pos)
            set(p->viewport_position, position());
    }
};
