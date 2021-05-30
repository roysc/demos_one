#pragma once
#include "mouse_core.hpp"

#include <Rxt/io.hpp>
#include <Rxt/math.hpp>
#include <SDL2/SDL.h>

#include <vector>
#include <stdexcept>
#include <utility>
#include <map>

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

    std::vector<M*> m_chain;

    auto at(key_type key)
    {
        for (auto& link: m_chain) {
            auto it = link.find(key);
            if (it != end(link))
                return it->second;
        }
        throw std::out_of_range(key);
    }
};

template <class M>
map_chain<M> chain_maps(M const& head, M const& next)
{
    return {{&head, &next}};
}

// Produces minimal available index within a range
struct index_registry
{
    unsigned m_max_count;
    unsigned m_next_valid = 0;
    std::vector<bool> m_gaps;

    index_registry(unsigned max_count)
        : m_max_count(max_count)
    {}

    // Get next available index
    unsigned next()
    {
        // no gaps
        if (m_next_valid == m_gaps.size()) {
            m_gaps.push_back(false);
            return m_next_valid++;
        }

        auto next = m_next_valid;
        m_gaps[m_next_valid] = false;
        // search ahead for next gap
        do { ++m_next_valid; }
        while (m_next_valid < m_gaps.size() &&
               !m_gaps[m_next_valid]);
        return next;
    }

    // Mark index available
    void release(unsigned ix)
    {
        m_gaps.at(ix) = true;
        if (ix < m_next_valid)
            m_next_valid = ix;
    }

    bool full() const
    {
        return m_next_valid >= m_max_count;
    }
};
