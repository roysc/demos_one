#include "plant.hpp"
#include <string>

extern "C" void step_state(void* c)
{
    sdl::em_advance<plant_app>(c);
}

int main(int argc, char* argv[])
{
    int seed = 42;
    if (argc > 1) {
        seed = std::stoi(argv[1]);
    }

    auto vpsize = plant_app::viewport_uvec{800};
    auto loop = sdl::make_looper(new plant_app(vpsize), step_state);
    loop();
    return 0;
}
