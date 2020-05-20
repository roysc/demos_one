#pragma once
#include "observable.hpp"

#include <glm/glm.hpp>
#include <type_traits>

using ivec = glm::ivec2;
using uvec = glm::uvec2;

// template <class P, class Obs>
// struct _viewport_base
// {
//     P _position {0};
//     Size max_scale; //grid_size
//     Size scale_factor {1};

//     const Size size_px {max_scale * scale_factor};
//     const float margin_size = .1;

//     Obs _hooks_;
//     auto hooks() { return _hooks_.hooks(); }

//     P position() const {return _position;}
//     void position(P p) {_position = p; _hooks_.notify_all(); }
// };

namespace tags { struct viewport {}; }

template <class GT>
struct _grid_viewport
{
    using P = typename GT::position_type;
    using Size = typename GT::size_type;
    using subject_tag = tags::viewport;

    Size max_scale; //grid_size
    Size scale_factor {1};
    P _position {0};
    const Size size_px {max_scale * scale_factor};
    const float margin_size = .1;

    observable<subject_tag> _hooks;
    auto& get_subject(subject_tag) { return _hooks; }

    template <class O>
    void set_router(O& obr) { obr.set_subject(subject_tag{}, _hooks); }

    P position() const {return _position;}
    void position(P p) {_position = p; _hooks(); }

    void scale(int exp)
    {
        // simulate zoom in/out by scaling down/up resp.; correct position to keep centered
        const Size min_scale{1};
        if (exp > 0) {
            if (scale_factor.x > min_scale.x && scale_factor.y > min_scale.y)
                scale_factor /= 2;
        } else {
            if (scale_factor.x < max_scale.x && scale_factor.y < max_scale.y)
                scale_factor *= 2;
        }
        _hooks();
    }

    void move(int dx, int dy)
    {
        _position += P{dx, dy};
        _hooks();
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
        if (dv != P{0}) {
            move(dv.x, dv.y);
            return true;
        }
        return false;
    }

    auto from_nds(float x, float y) const
    {
        return floor(glm::vec2(x, y) * glm::vec2(size_cells() / 2u));
    }

    glm::vec2 to_nds(P p) const { return glm::vec2(p) / glm::vec2(size_cells() / 2u); }
};

template <class GT>
using viewport = std::conditional_t<std::is_same_v<typename GT::position_type, ivec>,
                                    _grid_viewport<GT>,
                                    void>;
