#include "controls.hpp"
#include "reactive.hpp"

#include "atrium/geometry.hpp"
#include "atrium/rendering.hpp"
#include "atrium/interaction.hpp"

#include <Rxt/graphics/shader/colored_triangle_3D.hpp>
#include <Rxt/graphics/shader/solid_color_3D.hpp>
#include <Rxt/graphics/sdl.hpp>
#include <Rxt/graphics/gl.hpp>
#include <Rxt/graphics/camera.hpp>
#include <Rxt/graphics/color.hpp>

#include <Rxt/geometry/shapes.hpp>

#include <Rxt/math.hpp>
#include <Rxt/time.hpp>
#include <Rxt/util.hpp>
#include <Rxt/io.hpp>

#include <glm/glm.hpp>
#include <chrono>
#include <string>
#include <functional>
#include <vector>

using std::chrono::steady_clock;
using time_point = steady_clock::time_point;

using Rxt::print;
namespace gl = Rxt::gl;
namespace sdl = Rxt::sdl;
using triangle_program = Rxt::shader_programs::colored_triangle_3D;
using line_program = Rxt::shader_programs::solid_color_3D<GL_LINES>;

using atrium::mesh_data;
using atrium::mesh_colors;
using atrium::object_mesh;
using atrium::ux_data;
// using ux_data = adapt_reactive<atrium::ux_data>;

using uvec2 = glm::uvec2;
using fvec2 = glm::vec2;
using fvec3 = glm::vec3;

struct ui_traits
{
    using position_type = fvec2;
    using size_type = uvec2;
};

using cursor_type = adapt_reactive_crt<reactive_cursor, ui_traits>;
using camera_type = adapt_reactive_crt<Rxt::reactive_focus_cam>;

//wip
// using agent_registry = std::vector<int>;
using terrain_map = dense_map<int>;

struct dirt_app : public sdl::simple_gui
                , public sdl::input_handler<dirt_app, true>
{
    fvec3 const start_camera{1};

    bool quit = false;
    sdl::key_dispatcher keys;
    sdl::metronome metronome {Rxt::duration_fps<30>(1), [this] { return !should_quit(); }};
    // time_point last_draw_time;
    // bool draw_needed = true;

    triangle_program triangle_prog;
    triangle_program::data b_triangles {triangle_prog};
    line_program line_prog;
    line_program::data b_lines {line_prog};

    cursor_type cursor;
    camera_type camera{start_camera};

    mesh_data geom;
    mesh_colors colors;
    ux_data ux;
    hooks<> model_update;

    terrain_map terrain;

    dirt_app(uvec2);

    void step(SDL_Event);
    void draw();

    void handle_mouse_motion(SDL_MouseMotionEvent);
    void handle_should_quit() { quit = true; }
    void handle_key_down(SDL_Keysym k) { keys.press(k); }
    bool should_quit() const { return quit; }

    void _init_controls();
    void _init_observers();

    auto insert_mesh(object_mesh mesh, Rxt::rgb color)
    {
        auto index = geom.insert(mesh);
        colors.emplace(index, color);
        return index;
    }
};

extern "C" void step_state(void* c)
{
    sdl::step<dirt_app>(c);
}

int main(int argc, char* argv[])
{
    int seed = 42;
    if (argc > 1) {
        seed = std::stoi(argv[1]);
    }

    auto loop = sdl::make_looper(new dirt_app(uvec2(800)), step_state);
    loop();
    return 0;
}

dirt_app::dirt_app(uvec2 size)
    : simple_gui("plaza: dirt", size)
{
    _init_observers();
    _init_controls();

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glEnable(GL_CULL_FACE);

    object_mesh mesh;
    Rxt::make_cuboid(mesh, atrium::_g3d::Point{-.5, -.5, -.5}, {.5, .5, .5});
    insert_mesh(mesh, Rxt::colors::red);
    geom.index_triangles();

    model_update();
    camera.on_update();
}

template <Rxt::axis3 Axis>
void _orbit_camera(dirt_app* ctx, float d)
{
    ctx->camera.orbit(glm::angleAxis(d, Rxt::basis3<glm::vec3>(Axis)));
}

void dirt_app::_init_controls()
{
    using Ax = Rxt::axis3;
    float speed = 0.04;

    auto show_info = [this] {
        print("cursor={} camera={}\n", cursor.position(), camera.position());
    };
    auto show_hl = [this] {
        if (ux.highlight) {
            print("cursor({}) => {}\n", cursor.position(), ux.highlight->second);
        } else {
            print("cursor({})\n", cursor.position());
        }
    };
    auto reset_camera = [this] {
        camera.position(start_camera);
        camera.on_update();
    };

    keys.on_scan["Right"] = std::bind(&_orbit_camera<Ax::z>, this, +speed);
    keys.on_scan["Left"] = std::bind(&_orbit_camera<Ax::z>, this, -speed);
    keys.on_scan["Up"] = std::bind(&_orbit_camera<Ax::y>, this, +speed);
    keys.on_scan["Down"] = std::bind(&_orbit_camera<Ax::y>, this, -speed);
    keys.on_scan[","] = [=, this] { camera.forward(+speed); };
    keys.on_scan["."] = [=, this] { camera.forward(-speed); };
    keys.on_press["C-W"] = [this] { quit = true; };
    keys.on_press["I"] = show_info;
    keys.on_press["D"] = show_hl;
    keys.on_press["R"] = reset_camera;
}

void dirt_app::_init_observers()
{
    Pz_observe(camera.on_update) {
        auto m = camera.model_matrix();
        auto v = camera.view_matrix();
        set(triangle_prog->model_matrix, m);
        set(triangle_prog->view_matrix, v);
        set(triangle_prog->mvp_matrix, camera.projection_matrix() * v * m);
        set(triangle_prog->light_position, glm::vec3 {15, 10, 15});

        set(line_prog->mvp_matrix, camera.projection_matrix() * v * m);
    };

    Pz_observe(cursor.on_update) {
        using atrium::to_point;
        auto [source, dir] = Rxt::cast_ray(cursor.position(), camera);

        auto newhl = face_query(geom, atrium::_g3d::Ray{to_point(source), to_point(source + dir)});
        if (ux.highlight != newhl) {
            ux.highlight = newhl;
        }
    };

    Pz_observe(ux.highlight.on_update) {
        using namespace Rxt::colors;
        Rxt::rgb const axis_colors[3] {red, green, blue};

        b_lines.clear();
        for (unsigned i = 0; i < 3; ++i) {
            auto c = axis_colors[i];
            b_lines.push(Rxt::zero3<glm::vec3>, c);
            b_lines.push(Rxt::basis3<glm::vec3>(i), c);
        }

        render_ux(ux, geom, b_lines);
        b_lines.update();
    };

    Pz_observe(model_update) {
        b_triangles.clear();
        render_triangles(geom, colors, b_triangles);
        b_triangles.update();
    };
}

void dirt_app::step(SDL_Event event)
{
    do {
        handle_input(event);
    } while (SDL_PollEvent(&event));
    keys.scan();

    auto updates = {
        &cursor.on_update,
        &camera.on_update,
        &ux.highlight.on_update,
        &model_update
    };
    auto dirty = flush_all(updates);
    if (dirty) draw();
}

void dirt_app::draw()
{
    auto bg = Rxt::colors::black;
    glClearColor(bg.r, bg.g, bg.b, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    b_triangles.draw();
    // Draw indicator lines over model
    glClear(GL_DEPTH_BUFFER_BIT);
    b_lines.draw();

    SDL_GL_SwapWindow(window.get());
}

void dirt_app::handle_mouse_motion(SDL_MouseMotionEvent motion)
{
    auto [x, y] = sdl::nds_coords(*window, motion.x, motion.y);
    cursor.position({x, y});
}
