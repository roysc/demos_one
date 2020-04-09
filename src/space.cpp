#include "space.hpp"
#include "OpenSimplexNoise.hh"

image_data create_map(grid_size size, int seed)
{
    using glm::vec2;

    int width = size.x, height = size.y;

    OSN::Noise<4> osn4(seed);
    auto noise_4D = [&] (auto... args) { return osn4.eval(args...); };
    auto get_noise = [&] (vec2 c, float r) {
        return sample_clifford_torus(c.x, c.y, noise_4D, r);
    };

    image_data image(boost::extents[width][height]);
    for (int x = 0; x < width; ++x) {
        for (int y = 0; y < height; ++y) {
            vec2 c = vec2(x, y) / vec2(size);
            float sample = 0;
            for (int i = 0; i < 8; ++i) {
                sample += (1.f / (i+1)) * get_noise(c, 1 << i);
            }
            image[x][y] = Vec4u8(sample * 0xFF / 2);
        }
    }
    // image[0][0] = Vec4u8(0xFF, 0, 0, 0xFF); // red origin
    return image;
}
