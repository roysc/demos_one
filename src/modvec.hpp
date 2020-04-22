#pragma once

#include <glm/glm.hpp>
#include <array>

// A modulus-wrapped vector
template <unsigned... Mod>
struct modvec
{
    using IVec = glm::ivec2;
    using UVec = glm::uvec2;

    IVec _vec;
    const UVec modulus {Mod...};

    void _wrap()
    {
        _vec %= modulus;
        for (unsigned i = 0; i < IVec::length(); ++i) {
            if (_vec[i] < 0) _vec[i] += modulus[i];
        }
    }

    template <class... T>
    modvec(T... xs) : _vec{xs...} { _wrap(); }
    modvec(UVec::value_type x) : _vec{IVec::value_type(x)} { _wrap(); }
    modvec(UVec v) : _vec(v) { _wrap(); }

    modvec& operator=(UVec v) { _vec = v; _wrap(); return *this; }
    modvec& operator+=(IVec u) { _vec += u; _wrap(); return *this; }
    modvec& operator-=(IVec u) { _vec -= u; _wrap(); return *this; }
    modvec& operator*=(IVec u) { _vec*= u; _wrap(); return *this; }

    operator UVec () const
    {
        return UVec{_vec};
    }
};
