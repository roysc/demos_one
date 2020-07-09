#include "dirt.hpp"
#include "rendering.hpp"

#include <Rxt/math.hpp>
#include <Rxt/io.hpp>

#include <glm/gtc/epsilon.hpp>

using Rxt::print;

void dirt_app::_init_signals_ui()
{
    PZ_observe(cursor.on_update) {
        using namespace geometry;
        auto [source, dir] = Rxt::cast_ray(cursor.position(), camera);
        auto newsel = face_query(ray(to_point(source), to_point(source + dir)), geom);
        if (selected != newsel) {
            selected.emplace(newsel);
        }

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
    };

    PZ_observe(selected.on_update) {
        using namespace Rxt::colors;
        Rxt::rgb const axis_colors[3] {red, green, blue};

        b_overlines.clear();
        for (unsigned i = 0; i < 3; ++i) {
            auto c = axis_colors[i];
            b_overlines.push(Rxt::zero3<fvec3>, c);
            b_overlines.push(Rxt::basis3<fvec3>(i), c);
        }
        if (selected) {
            render_hl(*selected, geom, b_overlines, palette.at("hl"));
            if (auto others = face_ephem.find(*selected); others != end(face_ephem))
                render_hl(others->second, ephem, b_overlines, palette.at("hl_water"));
        }
        b_overlines.update();
    };

    PZ_observe(on_debug) {
        print("camera.pos={} .focus={} .up={}\n", camera.position(), camera.focus, camera.up);
        if (selected) {
            print("cursor({}) => {}\n", cursor.position(), selected->second);
        } else {
            print("cursor({})\n", cursor.position());
        }
        if (drag_origin) print("drag_origin = {}\n", drag_origin->pos);
    };
}
