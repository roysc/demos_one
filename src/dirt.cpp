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
    , map_size(16)
    , initial_camera(fvec3(8), fvec3(fvec2(map_size) / 4.f, 0))
    , camera(initial_camera)
    , palette(default_palette())
    // , metronome(Rxt::duration_fps<30>(1), [this] { return !is_stopped(); })
    , ui_viewport(uvec2(20))
{
    _init_signals_ui();
    _init_signals_model();
    _init_controls();
    set(ui_line_prog->mvp_matrix, glm::mat4(1));
    camera.on_update();

    terrain_map tm(map_size);
    auto scale = 0xFF;
    fill_noise(map_size, 42, [&](int x, int y, auto a) { tm.put({x, y}, a * scale / 2); });
    terrain.emplace(tm);

    e_debug = entities.create();

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glEnable(GL_CULL_FACE);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
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

void dirt_app::handle_drag(fvec2 dist_nds, camera_state cam_start)
{
    float mag = length(dist_nds);
    assert(dist_nds != fvec2(0));// degenerate
    auto perp_nds = fvec2(-dist_nds.y, dist_nds.x); // ccw
    auto perp_vs = fvec4(perp_nds, 0, 0);
    auto about_ms = normalize(Rxt::unview(perp_vs, cam_start));
    auto q_drag_ms = glm::angleAxis(-(mag) * Rxt::tau, about_ms);
    cam_start.orbit(q_drag_ms);
    camera.emplace(cam_start);
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
        &model_update,
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
    b_overlines.draw();
    b_uilines.draw();

    SDL_GL_SwapWindow(window.get());
}
