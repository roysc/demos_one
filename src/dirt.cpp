#include "dirt.hpp"

#include <Rxt/math.hpp>
#include <Rxt/time.hpp>
#include <Rxt/io.hpp>
#include <Rxt/geometry/shapes.hpp>

#include <glm/gtx/perpendicular.hpp>
#include <functional>
#include <string>
#include <iomanip>

using glm::ivec2;

dirt_app::dirt_app(const char* title, uvec2 size)
    : simple_gui(title, size)
    , map_size(16)
    , initial_camera(fvec3(8), fvec3(fvec2(map_size) / 4.f, 0))
    , camera(initial_camera)
    // , metronome(Rxt::duration_fps<30>(1), [this] { return !is_stopped(); })
    // , ui_viewport(uvec2(20))
{
    _init_ui();
    _init_controls();

    set(ui_line_prog->mvp_matrix, glm::mat4(1));
    camera.on_update();

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glEnable(GL_CULL_FACE);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
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
    };
    auto dirty =
        flush_all(updates) +
        flush_all(model_updates());
    if (dirty) draw();
}

void dirt_app::draw()
{
    auto draw_buf = [this] (std::string n, auto& p) {
        if (auto b = p.buf.ptr(n))
            b->draw();
    };

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    draw_buf("triangles", triangle_prog);
    draw_buf("lines", line_prog);

    {
    glEnable(GL_BLEND);
    glDisable(GL_CULL_FACE);
        // gl::enable_guard _blend{GL_BLEND};
        // gl::disable_guard _cull{GL_CULL};
        draw_buf("tris_txp", triangle_prog);
    glDisable(GL_BLEND);
    glEnable(GL_CULL_FACE);
    }

    // Draw indicator lines over model
    glClear(GL_DEPTH_BUFFER_BIT);
    draw_buf("over_lines_axes", line_prog);
    draw_buf("over_lines_hl", line_prog);
    draw_buf("points", point_prog);

    SDL_GL_SwapWindow(window.get());
}
