#pragma once

#include <Rxt/math.hpp>

#include <glm/glm.hpp>
#include <boost/multi_array.hpp>

template <class T>
struct dense_map
{
    using data_type = boost::multi_array<T, 2>;
    using key_type = glm::uvec2;

    data_type _data;

    dense_map() {}

    template <class Ext>
    dense_map(Ext size) : _data(boost::extents[size.x][size.y]) {}

    template <class Ext>
    auto resize(Ext size) { _data.resize(boost::extents[size.x][size.y]); }

    auto put(key_type i, T a) { return _data[i.x][i.y] = a; }

    template <class F>
    auto for_each(F&& f)
    {
        auto shape = _data.shape();
        for (unsigned y = 0; y < shape[1]; ++y) {
            for (unsigned x = 0; x < shape[0]; ++x) {
                f(key_type{x, y}, _data[x][y]);
            }
        }
    }
};
