#pragma once

#include <Rxt/color.hpp>

#include <map>
#include <string>

using color_palette = std::map<std::string, Rxt::rgba>;

inline color_palette default_palette()
{
    using namespace Rxt;
    color_palette p;
    p.emplace("hl", colors::white);
    p.emplace("bg", colors::black);
    p.emplace("soil", colors::soil);
    p.emplace("sand", colors::sand);
    p.emplace("water", colors::blue);
    p.emplace("hl_water", colors::blue);
    return p;
}
