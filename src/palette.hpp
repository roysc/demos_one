#pragma once

#include <Rxt/graphics/color.hpp>

#include <map>
#include <vector>
#include <string_view>

template <class M>
struct map_chain
{
    using key_type = typename M::key_type;
    using value_type = typename M::value_type;

    std::vector<M*> list;

    auto at(key_type key)
    {
        for (auto& head: list) {
            auto it = head.find(key);
            if (it != end(head))
                return it->second;
        }
        throw std::out_of_range(key);
    }
};

template <class M>
map_chain<M> chain_map(M const& head, M const& next)
{
    return {{head, next}};
}

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
