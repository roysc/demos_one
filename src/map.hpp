#pragma once

#include <Rxt/vec.hpp>
#include <boost/multi_array.hpp>

template <class T>
struct dense_grid
{
    using value_type = T;
    using data_type = boost::multi_array<T, 2>;
    using key_type = Rxt::uvec2;
    using shape_type = key_type;

    data_type _data;

    dense_grid() {}
    dense_grid(shape_type shape)
        : _data(boost::extents[shape.x][shape.y])
    {}

    shape_type shape() const
    {
        shape_type ret;
        for (int i = 0; i < ret.length(); ++i)
            ret[i] = _data.shape()[i];
        return ret;
    }

    auto& operator=(dense_grid const& that)
    {
        new(&_data) data_type(that._data);
        return *this;
    }

    void resize(shape_type shape)
    {
        _data.resize(boost::extents[shape.x][shape.y]);
    }

    auto at(key_type i) { return _data[i.x][i.y]; }
    void put(key_type i, T a) { _data[i.x][i.y] = a; }

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

template <class T>
using dense_grid2 = dense_grid<T>;

template <class T>
using dense_grid3 = void;
