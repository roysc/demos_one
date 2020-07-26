#pragma once
#include "mouse_core.hpp"

#include <Rxt/io.hpp>
#include <Rxt/math.hpp>
#include <SDL2/SDL.h>

#include <vector>
#include <stdexcept>
#include <utility>

template <unsigned ix, class Vec>
Vec invert(Vec v) { v[ix] = -v[ix]; return v; }

inline mouse_button mouse_button_from_sdl(SDL_MouseButtonEvent button)
{
    switch (button.button) {
    case SDL_BUTTON_LEFT: return mouse_button::left;
    case SDL_BUTTON_MIDDLE: return mouse_button::middle;
    case SDL_BUTTON_RIGHT: return mouse_button::right;
    default: return mouse_button::invalid;
    }
}

auto orbit_cam = [](auto& cam, auto axis, float d)
{
    auto about = Rxt::basis3<glm::vec3>(axis);
    cam->orbit(glm::angleAxis(d, about));
};

// Return inserted value, whether or not it existed
template <class C, class... Ts>
auto& get_or_emplace(C& c, Ts&&... a)
{
    auto [it, did] = c.emplace(std::forward<Ts>(a)...);
    return it->second;
}

template <class K, class V>
struct permissive_map
{
    using key_type = K;
    using value_type = V;
    std::map<K, V> _map;

    value_type& operator[](key_type k)
    {
        return get_or_emplace(_map, k, V{});
    }
};

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
