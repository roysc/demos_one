#include "dirt.hpp"
#include "rendering.hpp"

#include <Rxt/math.hpp>
#include <Rxt/io.hpp>

#include <glm/gtc/epsilon.hpp>

void dirt_app::_init_ui()
{
    using Rxt::print;

    PZ_observe(cursor.on_update) {
        // if (enable_drag_around)
        if (drag_origin) {
            auto [pos, cam] = *drag_origin;
            auto drag = cursor.position() - pos;
            auto threshold = 0.001f;
            if (!all(glm::epsilonEqual(drag, fvec2(0), threshold)))
                handle_drag(drag, cam);
        }
    };

    PZ_observe(camera.on_update) {
        auto m = camera.model_matrix();
        auto v = camera.view_matrix();
        set(triangle_prog->model_matrix, m);
        set(triangle_prog->view_matrix, v);
        set(triangle_prog->mvp_matrix, camera.projection_matrix() * v * m);
        set(triangle_prog->light_position, fvec3(15, 10, 15));

        set(line_prog->mvp_matrix, camera.projection_matrix() * v * m);
        set(point_prog->mvp_matrix, camera.projection_matrix() * v * m);
    };

    PZ_observe(camera.on_update) {
        using namespace Rxt::colors;
        Rxt::rgb const axis_colors[3] {red, green, blue};

        auto& b_overlines = line_prog.buf["over_lines_axes"];
        b_overlines.clear();
        for (unsigned i = 0; i < 3; ++i) {
            auto c = axis_colors[i];
            b_overlines.push(Rxt::zero3<fvec3>, c);
            b_overlines.push(Rxt::basis3<fvec3>(i), c);
        }
        b_overlines.update();
    };

    PZ_observe(on_debug) {
        print("camera.pos={} .focus={} .up={}\n", camera.position(), camera.focus, camera.up);
        if (drag_origin) print("drag_origin = {}\n", drag_origin->pos);
    };
}
