// #include "map.hpp"

#include <Rxt/math.hpp>

// using Vec4u8 = glm::tvec4<unsigned char, glm::highp>;
// using image_data = boost::multi_array<Vec4u8, 2>;

// struct noise_function
// {
//     int seed;
//     OSN::Noise<4> osn4{seed};
// };

// Out should be callable as out(i2, i2, float)
template <class Out>
struct sampler_range
{
    Out out;
    // int width, height;
    using size_type = Rxt::vec::ivec2;
    // Size of the domain in cells
    size_type size;

    sampler_range(Out o, size_type s) : out(o), size(s) {}

    struct sentinel {};
    struct item
    {
        // int x, y;
        size_type pos;
        sampler_range* _r;

        // Normalized position of sample cell within domain
        auto sample_position() const
        {
            using Rxt::vec::fvec2;
            return fvec2(pos.x, pos.y) / fvec2(_r->size);
        }

        void put(float sample)
        {
            _r->out(pos.x, pos.y, sample);
        }

        auto& operator++()
        {
            auto size = _r->size;
            ++pos.x;
            if (pos.x >= size.x) {
                pos.x = 0;
                ++pos.y;
            }
            return *this;
        }

        auto& operator*() {
            if (*this != sentinel())
                return *this;
            throw std::out_of_range(to_string(pos));
        }

        friend bool operator!=(item it, sentinel) {
            auto end = it._r->size;
            end.x = 0;
            return end != it.pos;
        }
    };
    // using value_type = item;

    item begin() { return item{size_type(0), this}; }
    sentinel end() { return {}; }
};

template <class T, class S>
sampler_range(T a, S) -> sampler_range<T>;

template <class Noise4, class T>
void fill_clifford_torus(Noise4 noise4, T out)
{
    using Rxt::vec::fvec2;

    const unsigned scale = 0xFF; // todo
    for (auto el: out) {
        fvec2 c = el.sample_position();
        float sample = 0;
        for (int i = 0; i < 8; ++i) {
            auto noise = Rxt::sample_clifford_torus(c.x, c.y, 1 << i, noise4);
            sample += (1.f / (i+1)) * noise;
        }
        el.put(sample);
    }
}

