#include "game_context.hpp"

#include <Rxt/graphics/sdl.hpp>

#include <string>

namespace sdl = Rxt::sdl;

extern "C" void step_state(void* c)
{
    sdl::step<game_context>(c);
}

int main(int argc, char** argv)
{
    int seed = 42;
    if (argc > 1) {
        seed = std::stoi(argv[1]);
    }
    auto context = new game_context(seed);
    auto loop = sdl::make_looper(context, step_state);
    loop();

    return 0;
}
