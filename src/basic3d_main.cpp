#include "basic3d.hpp"

extern "C" void step_state(void* c)
{
    sdl::em_advance<basic3d_app>(c);
}

int main(int argc, char* argv[])
{
    int seed = 42;
    if (argc > 1) {
        seed = std::stoi(argv[1]);
    }

    auto loop = sdl::make_looper(new basic3d_app(uvec2(800)), step_state);
    loop();
    return 0;
}
