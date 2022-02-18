#include "space.hpp"
#include "noise.hpp"

namespace zspace2
{
z2_stage::z2_stage(z2_space& uni)
    : _space(&uni)
    , _grid(uni._stage_size)
{
    uni.generate_stage(*this);
}

z2_stage::size_type z2_stage::size() const
{
    // return _space->_stage_size;
    return _grid.shape();
}

z2_space::z2_space(size_type ss, int seed)
    : _stage_size(ss)
    , _generator{unsigned(seed)}
    , _root{*this}
{
    auto depth = 1;
    if (depth > max_stage_depth)
        throw std::invalid_argument("depth > max_stage_depth");
    // generate_stage(_stage_size.emplace_back(*this));
    // generate_stage(_root);

    // _stages.emplace_back(*this);
}

void z2_space::generate_stage(z2_stage& out)
{
    using NoiseFunc = OSN::Noise<4>;
    // todo deterministic wrt. cell path?
    std::uniform_int_distribution<int> dist{0, 64};
    auto seed = dist(_generator.gen);
    NoiseFunc noise{seed};
    auto scale = 0xFF;
    auto put_2d = [&](int x, int y, auto a) { out.grid().put({x, y}, a * scale / 2); };
    // auto sampler = sampler_range(put_2d, out.size());
    // auto noise_4d = [&] (auto... args) { return noise.eval(args...); };
    // fill_clifford_torus(noise_4d, sampler);
    fill_clifford_torus([&](auto... args) { return noise.eval(args...); },
                        sampler_range(put_2d, out.size()));
}

// z2_stage* z2_space::get_stage(stage_index i)
// {
//     return nullptr;
// }
}
