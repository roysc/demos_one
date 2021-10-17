#include "dirt.hpp"
#include <string>

int main(int argc, char* argv[])
{
    int seed = 42;
    if (argc > 1) {
        seed = std::stoi(argv[1]);
    }

    auto vpsize = dirt_app::viewport_size_type{800};
    auto loop = sdl::make_looper(new dirt_app(vpsize));
    loop();
    return 0;
}
