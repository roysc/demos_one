#include "game_context.hpp"

#include <Rxt/graphics/sdl.hpp>

namespace sdl = Rxt::sdl;

extern "C" void step_state(void* c)
{
    sdl::step<game_context>(c);
}

int main()
{
    auto context = new game_context();
    auto loop = sdl::make_looper(context, step_state);
    loop();

    return 0;
}
