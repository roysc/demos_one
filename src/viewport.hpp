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

template <class Tr>
struct _viewport_ivec2
{
    uvec max_scale; //grid_size
    uvec scale_factor {1};
    ivec _position {0};
    const uvec size_px {max_scale * scale_factor};
    const float margin_size = .1;

    typename Tr::observable_type _hooks_;
    auto hooks() { return _hooks_.hooks(); }
    // auto subject(tag::viewport) { return _hooks_; }

    ivec position() const {return _position;}
    void position(ivec p) {_position = p; _hooks_.notify_all(); }

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
        _hooks_.notify_all();
    }

    void move(int dx, int dy)
    {
        _position += ivec{dx, dy};
        _hooks_.notify_all();
    }

    // size in number of cells
    uvec size_cells() const
    {
        return uvec(glm::vec2(size_pixels()) / glm::vec2(scale_factor));
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

// template <class Tr>
// struct _viewport_fvec2
// {
//     using position_type = typename Tr::vec_type;
//     using size_type = typename Tr::size_type;

//     position_type _position;
//     size_type scale_factor;
//     typename Tr::observable_type _hooks_;
//     auto hooks() { return _hooks_.hooks(); }

//     position_type position() const {return _position;}
//     void position(position_type p) {_position = p; _hooks_.notify_all(); }

//     void scale(int exp)
//     {
//         // simulate zoom in/out by scaling down/up resp.; correct position to keep centered
//         const size_type min_scale{1};
//         if (exp > 0) {
//             if (scale_factor.x > min_scale.x && scale_factor.y > min_scale.y)
//                 scale_factor /= 2;
//         } else {
//             if (scale_factor.x < max_scale.x && scale_factor.y < max_scale.y)
//                 scale_factor *= 2;
//         }
//         _hooks_.notify_all();
//     }

//     void move(float dx, float dy)
//     {
//         _position += position_type{dx, dy};
//         _hooks_.notify_all();
//     }

//     // // size in number of cells
//     // size_type size_cells() const
//     // {
//     //     return size_type(glm::vec2(size_px) / glm::vec2(scale_factor));
//     // }

//     size_type size_pixels() const { return size_px; }

//     bool edge_scroll(position_type cursor_position, int speed)
//     {
//         // (0,0) is center-screen, so offset it to the corner
//         // auto vpsize = size_pixels();
//         // auto offset_pos = to_nds(cursor_position + P(vpsize / 2u));
//         auto offset_pos = to_nds(cursor_position + 1);
//         P dv {0};

//         // std::cout << "edge_scroll? "
//         //           << "nds=" << offset_pos
//         for (unsigned i = 0; i < dv.length(); ++i) {
//             if (offset_pos[i] < margin_size) {
//                 dv[i] = -speed;
//                 // dv[dv.offset_pos < margin_size] = -speed;
//             } else if (offset_pos[i] + margin_size >= 2) {
//                 dv[i] = +speed;
//             }
//         }
//         if (dv != P{0}) {
//             move(dv.x, dv.y);
//             return true;
//         }
//         return false;
//     }

//     auto from_nds(float x, float y) const
//     {
//         return P{x, y};
//         // return floor(glm::vec2(x, y) * glm::vec2(size_cells() / 2u));
//     }

//     glm::vec2 to_nds(P p) const
//     {
//         return p;
//     }
// };

// namespace _det {}
template <class Tr>
struct _viewport_
{
    using type = std::conditional_t<
        std::is_same_v<typename Tr::vec_type, ivec>,
        _viewport_ivec2<Tr>,
        void
        // _viewport_fvec2<Tr>
        >;
};

template <class Tr>
using viewport = typename _viewport_<Tr>::type;
