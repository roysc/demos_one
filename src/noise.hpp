// #include "map.hpp"

#include <Rxt/math.hpp>

// using Vec4u8 = glm::tvec4<unsigned char, glm::highp>;
// using image_data = boost::multi_array<Vec4u8, 2>;

// struct noise_function
// {
//     int seed;
//     OSN::Noise<4> osn4{seed};
// };

template <class Out>
struct sampler_range
{
    Out out;
    // int width, height;
    using size_type = Rxt::ivec2;
    size_type size;
    struct item
    {
        // int x, y;
        size_type pos;
        sampler_range* _r;
        auto sample_position() const
        {
            return fvec2(pos.x, pos.y) / fvec2(_r->size);
        }
        void put(float sample)
        {
            _r->out(pos.x, pos.y, sample);
        }
        auto& operator++(int) {
            auto size = _r->size;
            ++pos.x;
            if (pos.x >= size.x) {
                pos.x = 0;
                ++pos.y;
            }
            return *this;
        }
        auto& operator*() { return *this; }
        // friend bool operator==(item that) { return pos == that.pos && _r == that._r; }
        friend bool operator==(item it, sentinel) { return any(lessThan(_r->size, it.pos)); }
    };
    // using value_type = item;
    struct sentinel {};

    item begin() { return item{size_type(0), this}; }
    sentinel end() { return {}; }
};

template <class T>
sampler_range(T a) -> sampler_range<T>;

template <class Put, class Noise4>
void fill_clifford_torus(Rxt::uvec2 size, Put&& put, Noise4 noise4)
{
    using Rxt::fvec2;

    const unsigned scale = 0xFF;
    int width = size.x, height = size.y;

    for (int x = 0; x < width; ++x) {
        for (int y = 0; y < height; ++y) {
            fvec2 c = fvec2(x, y) / fvec2(size);

            float sample = 0;
            for (int i = 0; i < 8; ++i) {
                auto noise = Rxt::sample_clifford_torus(c.x, c.y, 1 << i, noise4);
                sample += (1.f / (i+1)) * noise;
            }
            el.put(sample);
        }
    }
}
