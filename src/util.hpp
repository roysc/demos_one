#pragma once

#include <Rxt/io.hpp>
#include <Rxt/math.hpp>
#include <SDL2/SDL.h>

#include <map>
#include <stdexcept>
#include <utility>
#include <vector>

template <unsigned ix, class Vec>
Vec invert(Vec v)
{
    v[ix] = -v[ix];
    return v;
}

auto orbit_cam = [](auto& cam, auto axis, float d) {
    auto about = Rxt::basis3<glm::vec3>(axis);
    cam->orbit(glm::angleAxis(d, about));
};

// Set at index, expanding vector as needed
template <class T>
void set_and_resize(std::vector<T>& c, std::size_t i, T const& val)
{
    if (i >= c.size())
        c.resize(i + 1);
    c[i] = val;
}

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

    value_type& operator[](key_type k) { return get_or_emplace(_map, k, V{}); }
};

template <class M>
struct map_chain
{
    using key_type = typename M::key_type;
    using value_type = typename M::value_type;

    std::vector<M*> m_chain;

    auto at(key_type key)
    {
        for (auto& link : m_chain) {
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
    using index_type = unsigned;

    unsigned m_max_count;
    unsigned m_next_valid = 0;
    std::vector<bool> m_gaps;

    index_registry(unsigned max_count = 2 << 16)
        : m_max_count(max_count)
    {}

    // Get next available index
    index_type next()
    {
        // no gaps
        if (m_next_valid == m_gaps.size()) {
            m_gaps.push_back(false);
            return m_next_valid++;
        }

        auto next = m_next_valid;
        m_gaps[m_next_valid] = false;
        // search ahead for next gap
        do {
            ++m_next_valid;
        } while (m_next_valid < m_gaps.size() && !m_gaps[m_next_valid]);
        return next;
    }

    // Mark index available
    void release(index_type ix)
    {
        m_gaps.at(ix) = true;
        if (ix < m_next_valid)
            m_next_valid = ix;
    }

    bool full() const { return m_next_valid >= m_max_count; }
};
