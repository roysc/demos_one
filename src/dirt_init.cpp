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

        // print("camera:\npos={}\nup={}\n", camera.position(), camera.up);
    };

    PZ_observe(cursor.on_update) {
        using a3um::to_point;
        auto [source, dir] = Rxt::cast_ray(cursor.position(), camera);

        auto newhl = geom.face_query(a3um::ray{to_point(source), to_point(source + dir)});
        if (ux != newhl) {
            ux.emplace(newhl);
        }
    };

    PZ_observe(ux.on_update) {
        using namespace Rxt::colors;
        Rxt::rgb const axis_colors[3] {red, green, blue};

        b_uilines.clear();
        for (unsigned i = 0; i < 3; ++i) {
            auto c = axis_colors[i];
            b_uilines.push(Rxt::zero3<fvec3>, c);
            b_uilines.push(Rxt::basis3<fvec3>(i), c);
        }
        if (ux)
            render_ux(*ux, geom, b_uilines);
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
        object_mesh mesh, eph;
        face_to_space f2s;
        terrain.for_each([&](auto pos, auto& cell)
        {
            auto _quad = [pos](float elev, object_mesh& m) {
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
            f2s[face(hd, mesh)] = pos;

            if (elev < .5) {
                auto hdw = _quad(.5, eph);
            }
        });

        auto i = add_mesh(mesh, to_rgba(Rxt::colors::soil));
        face_spaces[i] = f2s;

        add_ephemeral(eph, to_rgba(Rxt::colors::blue, .7));
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
        if (ux) {
            print("cursor({}) => {}\n", cursor.position(), ux->second);
        } else {
            print("cursor({})\n", cursor.position());
        }
    };
}
