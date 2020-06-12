#include "ui.hpp"

#include <Rxt/graphics/shader/colored_triangle_3D.hpp>
// #include <Rxt/graphics/shader/solid_color_3D.hpp>

#include <Rxt/graphics/sdl.hpp>
#include <Rxt/graphics/gl.hpp>
#include <Rxt/graphics/camera.hpp>
#include <Rxt/time.hpp>
#include <Rxt/util.hpp>

#include <glm/glm.hpp>

// #include <chrono>
// #include <thread>
// #include <utility>

namespace gl = Rxt::gl;
namespace sdl = Rxt::sdl;
using triangle_program = Rxt::shader_programs::colored_triangle_3D;
// using line_program = Rxt::shader_programs::solid_color_3D<GL_LINES>;

using std::chrono::steady_clock;
using time_point = steady_clock::time_point;

using ivec2 = glm::ivec2;
using fvec2 = glm::vec2;
using fvec3 = glm::vec3;

struct ui_traits // for viewport
{
    using position_type = fvec2;
    using size_type = fvec2;
};

using cursor_type = observable_cursor<fvec2>;
using viewport_type = observable_viewport<ui_traits>;

struct dirt_app : public sdl::simple_gui
                , public sdl::input_handler<dirt_app, true>
{
    Rxt::focused_camera camera {fvec3(1)};
    sdl::key_dispatcher keys;
    bool quit = false;

    triangle_program triangle_prog;
    triangle_program::data b_triangles {triangle_prog};

    cursor_type cursor;
    viewport_type viewport;

    time_point last_render_time;
    sdl::metronome metronome {Rxt::duration_fps<30>(1), [this] { return !should_quit(); }};

    dirt_app(ivec v)
        : simple_gui("plaza: dirt", v)
        , viewport{v}
    {
        init_controls();
    }

    void step(SDL_Event);
    void draw() {}

    void handle_should_quit() { quit = true; }
    void handle_key_down(SDL_Keysym k) { keys.press(k); }
    bool should_quit() const { return quit; }

    void init_controls();
};

extern "C" void step_state(void* c)
{
    sdl::step<dirt_app>(c);
}

int main(int argc, char** argv)
{
    int seed = 42;
    if (argc > 1) {
        seed = std::stoi(argv[1]);
    }

    auto loop = sdl::make_looper(new dirt_app(ivec2(800)), step_state);
    loop();
    return 0;
}

void dirt_app::init_controls()
{

}

void dirt_app::step(SDL_Event event)
{
    do {
        handle_input(event);
    } while (SDL_PollEvent(&event));
    keys.scan();

    bool dirty = false;
    if (dirty) draw();
}
