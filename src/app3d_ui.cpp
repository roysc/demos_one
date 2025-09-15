#include "app3d.hpp"

#include <Rxt/color.hpp>
#include <Rxt/io.hpp>
#include <Rxt/math.hpp>
#include <Rxt/vec.hpp>

#include <glm/gtc/epsilon.hpp>

void basic_app3d::_init_ui()
{
    using Rxt::print;
    using Rxt::vec::fvec3;

    set(ui_line_prog->mvp_matrix, Rxt::vec::fmat4(1));

    cursor.on_update += [this] {
        if (drag_origin) {
            auto [pos, cam] = *drag_origin;
            auto drag = cursor.position() - pos;
            auto threshold = 0.001f;
            if (!all(glm::epsilonEqual(drag, cursor_position_type(0), threshold)))
                handle_drag(drag, cam);
        }
    };

    camera.on_update += [this] {
        auto m = camera.model_matrix();
        auto v = camera.view_matrix();
        set(triangle_prog->model_matrix, m);
        set(triangle_prog->view_matrix, v);
        set(triangle_prog->mvp_matrix, camera.projection_matrix() * v * m);
        set(triangle_prog->light_position, fvec3(15, 10, 15));

        set(line_prog->mvp_matrix, camera.projection_matrix() * v * m);
        set(point_prog->mvp_matrix, camera.projection_matrix() * v * m);
    };

    camera.on_update += [this] {
        using namespace Rxt::colors;
        Rxt::rgb const axis_colors[3]{red, green, blue};

        auto& b_overlines = line_prog.buf("over_lines_axes");
        b_overlines.clear();
        for (unsigned i = 0; i < 3; ++i) {
            auto c = axis_colors[i];
            b_overlines.push(Rxt::zero3<fvec3>, c);
            b_overlines.push(Rxt::basis3<fvec3>(i), c);
        }
        b_overlines.update();
    };

    on_debug += [this] {
        // using namespace Rxt::operators;
        // using Rxt::operators::operator<<;
        print("camera.pos={} .focus={} .up={}\n", camera.position(), camera.focus, camera.up);
        if (drag_origin)
            print("drag_origin = {}\n", drag_origin->pos);
    };
}
