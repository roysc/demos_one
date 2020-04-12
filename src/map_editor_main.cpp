#include "map_editor.hpp"

#include <Rxt/graphics/sdl.hpp>

#include <string>

extern "C" void step_state(void* c)
{
    Rxt::sdl::step<map_editor>(c);
}

int main(int argc, char** argv)
{
    int seed = 42;
    if (argc > 1) {
        seed = std::stoi(argv[1]);
    }
    auto context = new map_editor(seed);
    auto loop = Rxt::sdl::make_looper(context, step_state);
    loop();

    return 0;
}
