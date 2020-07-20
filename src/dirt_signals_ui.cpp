#include "dirt.hpp"
#include "rendering.hpp"

#include <Rxt/math.hpp>
#include <Rxt/io.hpp>

#include <glm/gtc/epsilon.hpp>

using Rxt::print;

void dirt_app::_init_signals_ui()
{
    PZ_observe(opts["highlight_face"].on_disable) {
        highlighted_faces.emplace();
        highlighted_faces.on_update();
    };
    PZ_observe(opts["highlight_vertex"].on_disable) {
        highlighted_vertices.clear();
        highlighted_vertices.on_update();
    };

    PZ_observe(cursor.on_update) {
        using namespace plaza_geom;
        auto [source, dir] = Rxt::cast_ray(cursor.position(), camera);

        if (opts["highlight_face"]) {
            auto newhl = face_query(ray(to_point(source), to_point(source + dir)), geom);
            highlighted_faces.emplace(newhl);
        }

        if (opts["highlight_vertex"]) {
            auto hl = vertex_query(ray(to_point(source), to_point(source + dir)), geom, 4);
            highlighted_vertices.clear();
            for (auto [p_vd, d]: hl)
                highlighted_vertices.push_back(p_vd.second);
            highlighted_vertices.on_update();
        }

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

        auto& b_overlines = buf("over_lines_axes", line_prog);
        b_overlines.clear();
        for (unsigned i = 0; i < 3; ++i) {
            auto c = axis_colors[i];
            b_overlines.push(Rxt::zero3<fvec3>, c);
            b_overlines.push(Rxt::basis3<fvec3>(i), c);
        }
        b_overlines.update();
    };

    PZ_observe(highlighted_faces.on_update) {
        auto& b_overlines = buf("over_lines_hl", line_prog);
        b_overlines.clear();
        if (highlighted_faces) {
            render_hl(*highlighted_faces, geom, b_overlines, palette.at("hl"));
            if (auto others = face_ephem.find(*highlighted_faces); others != end(face_ephem))
                render_hl(others->second, ephem, b_overlines, palette.at("hl_water"));
        }
        b_overlines.update();
    };

    PZ_observe(highlighted_vertices.on_update) {
        auto& b = buf("points", point_prog);
        b.clear();
        for (auto [oi, vd]: highlighted_vertices) {
            auto pos = get(CGAL::vertex_point, geom.sources[oi], vd);
            b.push(to_glm(pos), Rxt::colors::white);
        }
        b.update();
    };

    PZ_observe(on_debug) {
        print("camera.pos={} .focus={} .up={}\n", camera.position(), camera.focus, camera.up);
        if (highlighted_faces) {
            print("cursor({}) => {}\n", cursor.position(), highlighted_faces->second);
        } else {
            print("cursor({})\n", cursor.position());
        }
        if (drag_origin) print("drag_origin = {}\n", drag_origin->pos);
    };
}
