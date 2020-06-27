#include "dirt.hpp"
#include "noise.hpp"
#include "util.hpp"

#include <Rxt/io.hpp>
#include <Rxt/math.hpp>
#include <Rxt/geometry/shapes.hpp>
#include <Rxt/graphics/color.hpp>
#include <Rxt/data/graph.hpp>

#include <random>
#include <functional>
#include <string>

using Rxt::print;
using glm::ivec2;

namespace cpt
{
struct zpos { ivec2 r; };
struct realpos { fvec3 r; };
// struct vel { fvec3 dr; };

struct body
{
    struct vertex_t { using kind = Rxt::vertex_property_tag; };
    struct edge_t { using kind = Rxt::edge_property_tag; };
    using graph_type = Rxt::g_dl<Rxt::property<vertex_t, fvec3>,
                                 Rxt::property<edge_t, Rxt::rgb>>;
    graph_type graph;

    template <class Lines, class P>
    void render(Lines& lines, P offset)
    {
        auto pointpm = get(vertex_t{}, graph);
        auto edgepm = get(edge_t{}, graph);
        for (auto e: Rxt::to_range(edges(graph))) {
            auto color = edgepm[e];
            lines.push(pointpm[source(e, graph)] + offset , color);
            lines.push(pointpm[target(e, graph)] + offset, color);
        }
    }
};

body create_plant()
{
    body::graph_type g;
    auto seed = add_vertex(fvec3(0), g);
    auto sprout = add_vertex(fvec3(0,0,.25), g);
    auto e = add_edge(seed, sprout, Rxt::colors::green, g);
    return {g};
}

struct bioent
{
    int age = 0;
    void update(body& bod) { age += 1; }
};
}

void put_plant(entity_registry& r, ivec2 pos)
{
    using namespace cpt;
    auto e = r.create();

    r.emplace<zpos>(e, pos);
    r.emplace<body>(e, create_plant());
    Rxt::print("creating plant at {}\n", pos);
}

dirt_app::dirt_app(uvec2 size)
    : simple_gui("plaza: dirt", size)
{
    _init_observers();
    _init_controls();

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glEnable(GL_CULL_FACE);

    // object_mesh mesh;
    // Rxt::make_cuboid(mesh, a3um::_g3d::Point{-.5, -.5, -.5}, {.5, .5, .5});
    // insert_mesh(mesh, Rxt::colors::red);

    // the map
    uvec2 map_size(8);
    terrain_map tm(map_size);
    auto scale = 0xFF;
    fill_noise(map_size, 42, [&](int x, int y, auto a) { tm.put({x, y}, a * scale / 2); });
    terrain.emplace(tm);

    camera.on_update();
}

void dirt_app::_init_observers()
{
    PZ_observe(camera.on_update) {
        auto m = camera.model_matrix();
        auto v = camera.view_matrix();
        set(triangle_prog->model_matrix, m);
        set(triangle_prog->view_matrix, v);
        set(triangle_prog->mvp_matrix, camera.projection_matrix() * v * m);
        set(triangle_prog->light_position, fvec3 {15, 10, 15});

        set(line_prog->mvp_matrix, camera.projection_matrix() * v * m);
    };

    PZ_observe(cursor.on_update) {
        using a3um::to_point;
        auto [source, dir] = Rxt::cast_ray(cursor.position(), camera);

        auto newhl = geom.face_query(a3um::ray{to_point(source), to_point(source + dir)});
        if (ux != newhl) {
            ux.emplace(newhl);
        }
    };

    PZ_observe(ux.on_update) {
        using namespace Rxt::colors;
        Rxt::rgb const axis_colors[3] {red, green, blue};

        b_uilines.clear();
        for (unsigned i = 0; i < 3; ++i) {
            auto c = axis_colors[i];
            b_uilines.push(Rxt::zero3<fvec3>, c);
            b_uilines.push(Rxt::basis3<fvec3>(i), c);
        }
        if (ux)
            render_ux(*ux, geom, b_uilines);
        b_uilines.update();
    };

    PZ_observe(model_update) {
        b_triangles.clear();
        render_triangles(geom, colors, b_triangles);
        b_triangles.update();
    };

    PZ_observe(terrain.on_update) {
        object_mesh mesh;
        face_to_space f2s;
        terrain.for_each([&](auto pos, auto& cell) {
            auto x = pos.x, y = pos.y;
            auto elev = normalize_elevation(cell);
            a3um::point corners[4] = {
                {  x,   y, elev},
                {x+1,   y, elev},
                {x+1, y+1, elev},
                {  x, y+1, elev}
            };
            auto hd = CGAL::make_quad(corners[0], corners[1], corners[2], corners[3], mesh);
            f2s[face(hd, mesh)] = pos;
        });
        auto i = insert_mesh(mesh, Rxt::colors::violet);
        face_spaces[i] = f2s;

        geom.index_triangles();
        model_update();
    };

    PZ_observe(ent_update) {
        auto render_body = [this](auto pos, auto& bod)
        {
            auto elev = normalize_elevation(terrain.at(pos.r));
            bod.render(b_lines, fvec3(pos.r, elev) + fvec3(.5,.5,0));
        };
        b_lines.clear();
        entreg.view<cpt::zpos, cpt::body>().each(render_body);
        b_lines.update();
    };
}

void dirt_app::_init_controls()
{
    float speed = 0.04;

    auto show_info = [this] {
        print("cursor={} camera={}\n", cursor.position(), camera.position());
    };
    auto show_hl = [this] {
        if (ux) {
            print("cursor({}) => {}\n", cursor.position(), ux->second);
        } else {
            print("cursor({})\n", cursor.position());
        }
    };
    auto reset_camera = [this] {
        camera.emplace(start_camera_at);
    };

    using Ax = Rxt::axis3;
    keys.on_scan["Right"] = std::bind(orbit_cam, &camera, Ax::z, +speed);
    keys.on_scan["Left"] = std::bind(orbit_cam, &camera, Ax::z, -speed);
    keys.on_scan["Up"] = std::bind(orbit_cam, &camera, Ax::y, +speed);
    keys.on_scan["Down"] = std::bind(orbit_cam, &camera, Ax::y, -speed);
    keys.on_scan[","] = [=, this] { camera.forward(+speed); };
    keys.on_scan["."] = [=, this] { camera.forward(-speed); };
    keys.on_press["C-W"] = [this] { quit = true; };
    keys.on_press["I"] = show_info;
    keys.on_press["D"] = show_hl;
    keys.on_press["R"] = reset_camera;

    PZ_observe(input.on_quit) { quit = true; };
    PZ_observe(input.on_key_down, SDL_Keysym k) { keys.press(k); };
    PZ_observe(input.on_mouse_motion, SDL_MouseMotionEvent motion) {
        auto [x, y] = sdl::nds_coords(*window, motion.x, motion.y);
        cursor.position({x, y});
    };
    PZ_observe(input.on_mouse_wheel, SDL_MouseWheelEvent wheel) {
        if (wheel.y != 0)
            camera.forward(wheel.y);
        if (wheel.x != 0)
            camera.orbit(glm::angleAxis(speed, Rxt::basis3<fvec3>(Ax::z)));
    };
    PZ_observe(input.on_mouse_down, SDL_MouseButtonEvent button) {
        switch (button.button) {
        case SDL_BUTTON_LEFT: {
            if (ux) {
                auto [oi, fd] = *ux;
                ivec2 pos = face_spaces[oi][fd];
                assert(Rxt::point_within(pos, terrain.shape()));
                put_plant(entreg, pos);
                ent_update();
            }
            break;
        }
        case SDL_BUTTON_MIDDLE:
            // drag_origin = controls.cursor_position_world();
            break;
        }
    };
}

void dirt_app::advance(SDL_Event event)
{
    do {
        input.handle_input(event);
    } while (SDL_PollEvent(&event));
    keys.scan();

    auto updates = {
        &cursor.on_update,
        &camera.on_update,
        &ux.on_update,
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
    b_lines.draw();
    // Draw indicator lines over model
    glClear(GL_DEPTH_BUFFER_BIT);
    b_uilines.draw();

    SDL_GL_SwapWindow(window.get());
}

extern "C" void step_state(void* c)
{
    sdl::em_advance<dirt_app>(c);
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
