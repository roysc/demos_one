#pragma once

#include <Rxt/graphics/color.hpp>
#include <string>

namespace
{
using Rxt::rgb;

struct tile_info
{
    std::string description;
    Rxt::rgb color;
};

using tile_id = std::size_t;

const std::vector<tile_info> tiles = {
    tile_info {"empty", rgb {0}},
    tile_info {"stone", Rxt::colors::gray},
};

// struct {
//     std::vector<tile_info> _info = {
//         tile_info {"empty", rgb {0}},
//         tile_info {"stone", Rxt::colors::gray},
//     };

//     auto get_info(tile_id id)
//     {
//         return _info.at(id);
//     }
// } tiles;
}
