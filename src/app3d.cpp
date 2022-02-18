#include "app3d.hpp"

#include <Rxt/geometry/shapes.hpp>
#include <Rxt/io.hpp>
#include <Rxt/math.hpp>
#include <Rxt/util.hpp>

#include <functional>
#include <glm/gtx/perpendicular.hpp>
#include <iomanip>
#include <string>

using Rxt::vec::fvec3;
using Rxt::vec::fvec4;
using Rxt::vec::ivec2;

basic_app3d::basic_app3d(const char* title, viewport_size_type size)
    : simple_gui(title, size)
    , initial_camera(fvec3(1), fvec3(0))
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

void basic_app3d::reset_camera() { camera.emplace(initial_camera); };

void basic_app3d::handle_drag(cursor_position_type dist_nds, camera_state cam_start)
{
    float mag = length(dist_nds);
    assert(dist_nds != cursor_position_type(0));                   // degenerate
    auto perp_nds = cursor_position_type(-dist_nds.y, dist_nds.x); // ccw
    auto perp_vs = fvec4(perp_nds, 0, 0);
    auto about_ms = normalize(Rxt::unview(perp_vs, cam_start));
    auto q_drag_ms = glm::angleAxis(-(mag)*Rxt::tau, about_ms);
    cam_start.orbit(q_drag_ms);
    camera.emplace(cam_start);
}

Rxt::hooks<> basic_app3d::_updates(SDL_Event event)
{
    do {
        input.handle_input(event);
    } while (SDL_PollEvent(&event));
    keys().scan();

    return Rxt::make_hooks(cursor.on_update, camera.on_update);
}

void basic_app3d::draw()
{
    auto draw_buf = [this](std::string n, auto& p) {
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

    SDL_GL_SwapWindow(window());
}
