#include "dirt.hpp"
#include "rendering.hpp"

#include <Rxt/math.hpp>
#include <Rxt/io.hpp>

using Rxt::print;

void dirt_app::_init_signals_ui()
{
    PZ_observe(cursor.on_update) {
        using a3um::to_point;
        auto [source, dir] = Rxt::cast_ray(cursor.position(), camera);

        auto newsel = geom.face_query(a3um::ray{to_point(source), to_point(source + dir)});
        if (selected != newsel) {
            selected.emplace(newsel);
        }

        if (drag_origin) {
            auto drag = cursor.position() - drag_origin->pos;
            handle_drag(drag);
            // print("dragging from {}, dist = {}\n", drag_origin->pos, drag);
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

        b_uilines.clear();
        for (unsigned i = 0; i < 3; ++i) {
            auto c = axis_colors[i];
            b_uilines.push(Rxt::zero3<fvec3>, c);
            b_uilines.push(Rxt::basis3<fvec3>(i), c);
        }
        if (selected) {
            render_hl(*selected, geom, b_uilines, palette.at("hl"));
            if (auto others = face_ephem.find(*selected); others != end(face_ephem))
                render_hl(others->second, ephem, b_uilines, palette.at("hl_water"));
        }
        b_uilines.update();
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
