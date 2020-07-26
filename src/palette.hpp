#pragma once

#include <Rxt/graphics/color.hpp>

#include <map>
#include <string>

using color_palette = std::map<std::string, Rxt::rgba>;

inline color_palette default_palette()
{
    using namespace Rxt;
    color_palette p;
    p.emplace("hl", to_rgba(colors::white));
    p.emplace("bg", to_rgba(colors::black));
    p.emplace("soil", to_rgba(colors::soil));
    p.emplace("sand", to_rgba(colors::sand));
    p.emplace("water", to_rgba(colors::blue));
    p.emplace("hl_water", to_rgba(colors::blue));
    return p;
}
