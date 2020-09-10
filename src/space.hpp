#pragma once

#include "map.hpp"
#include "stage.hpp"

#include <Rxt/vec.hpp>
#include <memory>
#include <random>

using transform3 = Rxt::fmat4;

inline Rxt::fvec3 apply(transform3 m, Rxt::fvec3 v)
{
    return Rxt::fvec3(m * Rxt::fvec4(v, 1));
}

#define PZ_use_traits(_traits_t)                    \
    using traits = _traits_t;                       \
    using position_type = typename _traits_t::position; \
    using size_type = typename _traits_t::size

// index integer-positions ie. grid objects
namespace zspace2
{
struct spatial_traits
{
    using position = Rxt::ivec2; // could be unsigned (torus)
    using velocity = Rxt::ivec2;
    using size = Rxt::uvec2;

    enum direction : unsigned char { w, e, n, s };
    static constexpr auto to_velocity(direction dir)
    {
        velocity _deltas[4] = {
            {-1, 0}, 
            {+1, 0}, 
            {0, -1}, 
            {0, +1},
        };
        return _deltas[static_cast<unsigned>(dir)];
    }
};

struct z2_generator
{
    unsigned seed;
    std::default_random_engine gen{seed};
};

struct z2_space;

struct z2_stage
{
    PZ_use_traits(spatial_traits);

    using cell_value = std::uint8_t;
    using terrain_grid = dense_grid<cell_value>;

    z2_space* _space;
    terrain_grid _grid;

    z2_space& space() { return *_space; }
    size_type size() const;
    auto& grid() { return _grid; }

    z2_stage(z2_space&);
};

struct z2_deep_space
{
    PZ_use_traits(spatial_traits);

    using stage_type = z2_stage;
    using node_type = deep_stage<stage_type>;
    const unsigned max_stage_depth = 2;
    const size_type _stage_size;
    const size_type _full_size = _stage_size << max_stage_depth;

    z2_generator _generator;
    // PIMPL?
    // index w/ morton code?
    node_type _root;

    z2_deep_space(size_type, int);
    node_type* root() { return &_root; }
    z2_generator& generator() { return _generator; }
    size_type full_size() const { return _full_size; }
    size_type stage_size() const { return _stage_size; }

    // stage_type* get_stage(stage_index);
    void generate_stage(stage_type&);
};
}
