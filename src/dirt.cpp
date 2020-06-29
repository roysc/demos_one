#include "dirt.hpp"
#include "noise.hpp"
#include "util.hpp"

#include <Rxt/io.hpp>
#include <Rxt/math.hpp>
#include <Rxt/geometry/shapes.hpp>
#include <Rxt/graphics/color.hpp>

#include <functional>
#include <string>

using Rxt::print;
using Rxt::to_rgba;
using glm::ivec2;

static float normalize_elevation(terrain_value t)
{
    auto max_elev = std::numeric_limits<terrain_value>::max();
    return float(t)/max_elev;
}

dirt_app::dirt_app(uvec2 size)
    : simple_gui("plaza: dirt", size)
{
    _init_observers();
    _init_controls();

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glEnable(GL_CULL_FACE);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    uvec2 map_size(8);
    terrain_map tm(map_size);
    auto scale = 0xFF;
    fill_noise(map_size, 42, [&](int x, int y, auto a) { tm.put({x, y}, a * scale / 2); });
    terrain.emplace(tm);

    camera.focus = fvec3(fvec2(map_size) / 4.f, 0);
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

        // print("camera:\npos={}\nup={}\n", camera.position(), camera.up);
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

        b_tris_alpha.clear();
        render_triangles(ephem, ephem_colors, b_tris_alpha);
        b_tris_alpha.update();
    };

    PZ_observe(terrain.on_update) {
        object_mesh mesh, eph;
        face_to_space f2s;
        terrain.for_each([&](auto pos, auto& cell)
        {
            auto _quad = [pos](float elev, object_mesh& m) {
                auto x = pos.x, y = pos.y;
                a3um::point corners[4] = {
                    {  x,   y, elev},
                    {x+1,   y, elev},
                    {x+1, y+1, elev},
                    {  x, y+1, elev}
                };
                return make_quad(corners[0], corners[1], corners[2], corners[3], m);
            };
            auto elev = normalize_elevation(cell);
            auto hd = _quad(elev, mesh);
            f2s[face(hd, mesh)] = pos;

            if (elev < .5) {
                auto hdw = _quad(.5, eph);
            }
        });

        auto i = add_mesh(mesh, to_rgba(Rxt::colors::soil));
        face_spaces[i] = f2s;

        add_ephemeral(eph, to_rgba(Rxt::colors::blue, .7));
    };

    PZ_observe(ent_update) {
        auto render_body = [this](auto pos, auto& bod)
        {
            auto elev = normalize_elevation(terrain.at(pos.r));
            bod.render(b_lines, fvec3(pos.r, elev) + fvec3(.5,.5,0));
        };
        b_lines.clear();
        entreg.view<cpt::zpos, cpt::skel>().each(render_body);
        b_lines.update();
    };

    PZ_observe(on_debug) {
        print("camera.pos={} .focus={} .up={}\n", camera.position(), camera.focus, camera.up);
        if (ux) {
            print("cursor({}) => {}\n", cursor.position(), ux->second);
        } else {
            print("cursor({})\n", cursor.position());
        }
    };
}

void dirt_app::_init_controls()
{
    float speed = 0.04;

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
    keys.on_press["D"] = on_debug;
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
            if (auto pos = selected_space()) {
                put_body(entreg, *pos, cpt::build_plant());
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

    {
    glEnable(GL_BLEND);
    glDisable(GL_CULL_FACE);
        // gl::enable_guard _blend{GL_BLEND};
        // gl::disable_guard _cull{GL_CULL};
        b_tris_alpha.draw();
    glDisable(GL_BLEND);
    glEnable(GL_CULL_FACE);
    }

    // Draw indicator lines over model
    glClear(GL_DEPTH_BUFFER_BIT);
    b_uilines.draw();

    SDL_GL_SwapWindow(window.get());
}
