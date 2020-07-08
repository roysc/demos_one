#pragma once
#include "_debug.hpp"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/transform.hpp>
#include <glm/glm.hpp>
#include <type_traits>

using ivec = glm::ivec2;
using uvec = glm::uvec2;
using fvec = glm::fvec2;

template <class ST>
struct basic_viewport
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

    basic_viewport(Size max, Size scale = Size(1))
        : max_scale{max}, scale_factor{scale}
    {
        assert(all(lessThan(Size(0), scale)));
        assert(all(lessThan(Size(0), max)));
    }

    virtual void position(P pos) { _position = pos; }
    virtual void set_scale(Size scale) { scale_factor = scale; }

    P position() const { return _position; }

    Size scale_relative(float factor) const
    {
        return scale_factor * factor;
    }

    Size scale_pow(int exp) const
    {
        const Size min_scale{1};
        if (exp > 0) {
            if (scale_factor.x > min_scale.x && scale_factor.y > min_scale.y)
                return scale_factor / 2u;
        } else {
            if (scale_factor.x < max_scale.x && scale_factor.y < max_scale.y)
                return scale_factor * 2u;
        }
        return scale_factor;
    }

    // size in number of cells
    Size size_cells() const
    {
        return Size(fvec(size_pixels()) / fvec(scale_factor));
    }

    Size size_pixels() const { return size_px; }

    auto from_nds(float x, float y) const
    {
        return floor(fvec(x, y) * fvec(size_cells() / 2u));
    }

    fvec to_nds(P p) const { return fvec(p) / fvec(size_cells() / 2u); }

    glm::mat4 view_matrix() const
    {
        using glm::vec3;

        auto pos3 = vec3(position(), 0);
        auto cells = vec3(size_cells(), 1);
        glm::mat4 view_matrix =
            glm::scale(vec3(2.f / fvec(cells), 0)) *
            glm::translate(-pos3);
        return view_matrix;
    }

    glm::mat4 projection_matrix() const
    {
        // L R B T N F
        auto pos = position();
        auto corner = position() + size_cells();
        return glm::ortho(pos.x, corner.x, pos.y, corner.y);
    }

    glm::mat4 model_matrix() const
    {
        // return glm::translate(glm::vec3(.5, .5, 0)); // offset to center of cell
        return glm::mat4{1};
    }

    void translate(P d)
    {
        position(position() + d);
    }

    // scale, adjusting to maintain position
    void scale_to(Size coef, P focw)
    {
        ivec max {max_scale};
        auto coefr = fvec(coef) / fvec(max);
        P newpos = P(coefr * fvec(position() - focw)) + focw;
        position(newpos);
        set_scale(coef);
        // PZ_debug("coef = {}, rel. = {}\n", coef, coefr);
    }

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
        if (dv != P(0)) {
            translate(dv);
            return true;
        }
        return false;
    }

    template <class P>
    void update_uniforms(P& p, bool pos = true)
    {
        set(p->viewport_size, size_cells());
        if (pos)
            set(p->viewport_position, position());
    }
};
