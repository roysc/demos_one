#include "dirt.hpp"
#include "noise.hpp"

#include <Rxt/math.hpp>
#include <Rxt/time.hpp>
#include <Rxt/io.hpp>
#include <Rxt/geometry/shapes.hpp>

#include <glm/gtx/perpendicular.hpp>
#include <functional>
#include <string>
#include <iomanip>

using glm::ivec2;

dirt_app::dirt_app(uvec2 size)
    : simple_gui("plaza: dirt", size)
    , palette(default_palette())
    // , metronome(Rxt::duration_fps<30>(1), [this] { return !is_stopped(); })
    , initial_camera{fvec3(8), fvec3()}
    , camera{initial_camera}
{
    _init_signals_ui();
    _init_signals_model();
    _init_controls();

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glEnable(GL_CULL_FACE);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    uvec2 map_size(16);
    terrain_map tm(map_size);
    auto scale = 0xFF;
    fill_noise(map_size, 42, [&](int x, int y, auto a) { tm.put({x, y}, a * scale / 2); });
    terrain.emplace(tm);

    camera.focus = fvec3(fvec2(map_size) / 4.f, 0);
    camera.on_update();

    // std::cout << std::setprecision(1);
    e_debug = entities.create();
}

std::optional<ivec2> dirt_app::selected_space() const
{
    if (selected) {
        auto [oi, fd] = *selected;
        ivec2 pos = face_spaces.at(oi).at(fd);
        assert(Rxt::point_within(pos, terrain.shape()));
        return pos;
    }
    return std::nullopt;
}

void dirt_app::handle_drag(fvec2 nds)
{
    using Rxt::print;
    auto p_vs = Rxt::unproject(fvec4(nds, 0, 0), camera);
    float dist = length(nds) * Rxt::tau;
    // auto perp = perp(fvec2(1), nds);
    auto perp = fvec4(-nds.y, nds.x, 0, 0);
    auto about = Rxt::unview(perp, camera);
    auto q_drag = glm::angleAxis(glm::degrees(dist), about);
    auto newcam = drag_origin->cam;
    newcam.orbit(q_drag);
    camera.emplace(newcam);

    {
    skeleton g;
    // auto b = g.builder(); b.e[b.v(p), b.v(pp), white];
    add_edge(add_vertex(fvec3(0), g.graph), add_vertex(10.f * about, g.graph),
             Rxt::colors::white, g.graph);
    // e_debug = put_body(entities, camera.focus, g);
    entities.emplace<cpt::skel>(e_debug, g);
    entities.emplace<cpt::fpos>(e_debug, camera.focus);
    ent_update();
    }
    print("drag perp = {} about = {} dist = {}\n", perp, about, dist);
    // print("drag about = {} dist = {}\n", about, dist);
    // print("camera hooks #= {}\n", camera.on_update.size());
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
        &selected.on_update,
        &model_update
    };
    auto dirty = flush_all(updates);
    if (dirty) draw();
}

void dirt_app::draw()
{
    auto bg = palette.at("bg");
    glClearColor(bg.r, bg.g, bg.b, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    b_triangles.draw();
    b_lines.draw();

    {
    glEnable(GL_BLEND);
    glDisable(GL_CULL_FACE);
        // gl::enable_guard _blend{GL_BLEND};
        // gl::disable_guard _cull{GL_CULL};
        b_tris_txp.draw();
    glDisable(GL_BLEND);
    glEnable(GL_CULL_FACE);
    }

    // Draw indicator lines over model
    glClear(GL_DEPTH_BUFFER_BIT);
    b_uilines.draw();

    SDL_GL_SwapWindow(window.get());
}
