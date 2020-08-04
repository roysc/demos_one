#include "atrium.hpp"

#include <Rxt/math.hpp>
#include <Rxt/time.hpp>
#include <Rxt/io.hpp>
#include <Rxt/geometry/shapes.hpp>

#include <glm/gtx/perpendicular.hpp>
#include <functional>
#include <string>
#include <iomanip>

using Rxt::ivec2;
using Rxt::fvec3;
using Rxt::fvec4;

atrium_app::atrium_app(const char* title, viewport_uvec size)
    : simple_gui(title, size)
    , initial_camera(fvec3{1}, fvec3{0})
    , camera(initial_camera)
    // , metronome(Rxt::duration_fps<30>(1), [this] { return !is_stopped(); })
    // , ui_viewport(uvec2(20))
{
    _init_ui();
    _init_controls();

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glEnable(GL_CULL_FACE);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void atrium_app::handle_drag(cursor_fvec dist_nds, camera_state cam_start)
{
    float mag = length(dist_nds);
    assert(dist_nds != cursor_fvec(0));// degenerate
    auto perp_nds = cursor_fvec(-dist_nds.y, dist_nds.x); // ccw
    auto perp_vs = fvec4(perp_nds, 0, 0);
    auto about_ms = normalize(Rxt::unview(perp_vs, cam_start));
    auto q_drag_ms = glm::angleAxis(-(mag) * Rxt::tau, about_ms);
    cam_start.orbit(q_drag_ms);
    camera.emplace(cam_start);
}

Rxt::reactive_handle atrium_app::_update(SDL_Event event)
{
    do {
        input.handle_input(event);
    } while (SDL_PollEvent(&event));
    keys.scan();

    return {
        &cursor.on_update,
        &camera.on_update,
    };
}

void atrium_app::draw()
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
