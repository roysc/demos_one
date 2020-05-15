#include "canvas.hpp"
#include <Rxt/range.hpp>
#include <iostream>

namespace gl = Rxt::gl;
namespace sdl = Rxt::sdl;
using Rxt::print;

extern "C" void step_state(void* c)
{
    sdl::step<canvas>(c);
}

int main(int argc, char** argv)
{
    int seed = 42;
    if (argc > 1) {
        seed = std::stoi(argv[1]);
    }

    try {
        auto loop = sdl::make_looper(
            new canvas(grid_viewport{uvec(640)}),
            step_state
        );
        loop();
    } catch (std::exception& e) {
        std::cout << "caught exception: " << e.what() << '\n';
    }

    return 0;
}

canvas::canvas(grid_viewport vp)
    : simple_gui("plaza: canvas", vp.size_pixels())
    , viewport{vp}
    , tool(&selector)
{
    viewport.hook([&](auto& gv) { ui.set_viewport(gv); set_dirty(); });
    cursor.hook([&](auto& c) { ui.set_cursor(c.position, c.size); set_dirty(); });
    selector.hook([&](auto& s) {
        for (auto [a, b]: Rxt::to_range(s.selection)) {
            ui.set_selection(a, b);
        }
        set_dirty();
    });
    cursor.hook([&](auto& c){ std::cout << "cursor=" << c.position << "\n"; });

    // viewport.hook([&](auto& gv) {
    //                   ui.set_viewport(gv);
    //                   set_dirty();
    //               });
    viewport.hooks() += (auto& gv) {
        ui.set_viewport(gv);
        set_dirty();
    };
    cursor.hook([&](auto& c) {
                    ui.set_cursor(c.position, c.size);
                    set_dirty();
                });
    selector.hook([&](auto& s) {
        for (auto [a, b]: Rxt::to_range(s.selection)) {
            ui.set_selection(a, b);
        }
        set_dirty();
    });

    set_dirty();
    glClearColor(0, 0, 0, 1);
}
