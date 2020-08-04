#pragma once

#include "map.hpp"
#include <Rxt/vec.hpp>

#include <memory>

// index integer-positions ie. grid objects
namespace zspace2
{
// struct zspace2_traits
// {
using position_type = Rxt::ivec2;
using size_type = Rxt::uvec2;
// };

struct z2_generator
{
    int seed;
};

struct z2_universe;

struct z2_stage
{
    using cell_value = std::uint8_t;
    using terrain_grid = dense_grid<cell_value>;
    // using terrain_map = Rxt::adapt_reactive<geog_grid>;

    z2_universe* _universe;
    terrain_grid _grid;
    // z-order coord?

    size_type size() const;
    auto& grid() { return _grid; }

    z2_stage(z2_universe&);
};

template <class Stage>
struct deep_stage;

template <class Stage>
struct deep_stage
    : Stage
{
    using super_type = Stage;
    using pointer = std::unique_ptr<deep_stage>;
    dense_grid<pointer> substages;
    template <class U>
    deep_stage(U& uni)
        : super_type(uni)
        , substages{this->size()}
    {}
};

struct z2_universe
{
    const unsigned max_stage_depth = 2;
    const size_type _stage_size;
    const size_type _full_size = _stage_size << max_stage_depth;

    z2_generator _generator;
    // std::vector<z2_stage> _stage_cache;
    deep_stage<z2_stage> _root;

    z2_universe(size_type, int);
    z2_stage* root() { return &_root; }
    z2_generator& generator() { return _generator; }
    size_type full_size() const { return _full_size; }
    size_type stage_size() const { return _stage_size; }

    // z2_stage* get_stage(stage_index);
    void generate_stage(z2_stage&);
};
}
