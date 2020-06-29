#include "dirt.hpp"
#include <Rxt/graphics/color.hpp>
#include <Rxt/io.hpp>

using Rxt::print;
using Rxt::to_rgba;

static float normalize_elevation(terrain_value t)
{
    auto max_elev = std::numeric_limits<terrain_value>::max();
    return float(t)/max_elev;
}

void dirt_app::_init_observers()
{
    PZ_observe(camera.on_update) {
        auto m = camera.model_matrix();
        auto v = camera.view_matrix();
        set(triangle_prog->model_matrix, m);
        set(triangle_prog->view_matrix, v);
        set(triangle_prog->mvp_matrix, camera.projection_matrix() * v * m);
        set(triangle_prog->light_position, fvec3 {15, 10, 15});

        set(line_prog->mvp_matrix, camera.projection_matrix() * v * m);
    };

    PZ_observe(cursor.on_update) {
        using a3um::to_point;
        auto [source, dir] = Rxt::cast_ray(cursor.position(), camera);

        auto newhl = geom.face_query(a3um::ray{to_point(source), to_point(source + dir)});
        if (hlite != newhl) {
            hlite.emplace(newhl);
        }
    };

    PZ_observe(hlite.on_update) {
        using namespace Rxt::colors;
        Rxt::rgb const axis_colors[3] {red, green, blue};

        b_uilines.clear();
        for (unsigned i = 0; i < 3; ++i) {
            auto c = axis_colors[i];
            b_uilines.push(Rxt::zero3<fvec3>, c);
            b_uilines.push(Rxt::basis3<fvec3>(i), c);
        }
        if (hlite) {
            render_hl(*hlite, geom, b_uilines, Rxt::colors::white);
            if (auto others = face_ephem.find(*hlite); others != end(face_ephem))
                render_hl(others->second, ephem, b_uilines, Rxt::colors::sky_blue);
        }
        b_uilines.update();
    };

    PZ_observe(model_update) {
        b_triangles.clear();
        render_triangles(geom, colors, b_triangles);
        b_triangles.update();

        b_tris_alpha.clear();
        render_triangles(ephem, ephem_colors, b_tris_alpha);
        b_tris_alpha.update();
    };

    PZ_observe(terrain.on_update) {
        a3um::mesh mesh, eph;
        face_to_space f2s;
        std::map<object_face_descriptor, object_face_descriptor> f2f;

        terrain.for_each([&](auto pos, auto& cell)
        {
            auto _quad = [pos](float elev, auto& m) {
                auto x = pos.x, y = pos.y;
                a3um::point corners[4] = {
                    {  x,   y, elev},
                    {x+1,   y, elev},
                    {x+1, y+1, elev},
                    {  x, y+1, elev}
                };
                return make_quad(corners[0], corners[1], corners[2], corners[3], m);
            };
            auto elev = normalize_elevation(cell);
            auto hd = _quad(elev, mesh);
            auto fd = face(hd, mesh);
            f2s[fd] = pos;

            if (elev < .5) {
                auto hdw = _quad(.5, eph);
                f2f[fd] = face(hdw, eph);
            }
        });

        auto i = add_mesh(mesh, to_rgba(Rxt::colors::soil));
        face_spaces[i] = f2s;

        auto ei = add_ephemeral(eph, to_rgba(Rxt::colors::blue, .7));
        for (auto& [f, ef]: f2f) {
            face_ephem[object_face(i, f)] = object_face(ei, ef);
        }
    };

    PZ_observe(ent_update) {
        auto render_body = [this](auto pos, auto& bod)
        {
            auto elev = normalize_elevation(terrain.at(pos.r));
            bod.render(b_lines, fvec3(pos.r, elev) + fvec3(.5,.5,0));
        };
        b_lines.clear();
        entreg.view<cpt::zpos, cpt::skel>().each(render_body);
        b_lines.update();
    };

    PZ_observe(on_debug) {
        print("camera.pos={} .focus={} .up={}\n", camera.position(), camera.focus, camera.up);
        if (hlite) {
            print("cursor({}) => {}\n", cursor.position(), hlite->second);
        } else {
            print("cursor({})\n", cursor.position());
        }
    };
}
