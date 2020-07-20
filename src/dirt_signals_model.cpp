#include "dirt.hpp"
#include "rendering.hpp"
#include <Rxt/graphics/color.hpp>

using Rxt::to_rgba;

template <class I>
static float normalize_int(I t)
{
    I max_elev = std::numeric_limits<I>::max();
    return float(t)/max_elev;
}

void dirt_app::_init_signals_model()
{
    // auto& b_uilines = buf("uilines", ui_line_prog);
    auto& b_lines = buf("lines", line_prog);
    auto& b_overlines = buf("overlines", line_prog);
    auto& b_triangles = buf("triangles", triangle_prog);
    auto& b_tris_txp = buf("tris_txp", triangle_prog);

    PZ_observe(model_update) {
        b_triangles.clear();
        render_triangles(geom, colors, b_triangles);
        b_triangles.update();

        b_tris_txp.clear();
        render_triangles(ephem, ephem_colors, b_tris_txp);
        b_tris_txp.update();
    };

    PZ_observe(terrain.on_update) {
        using Sfd = mesh_data::source_face_descriptor;
        mesh3 mesh, eph;
        face_to_space f2s;
        std::map<Sfd, Sfd> f2f;

        terrain.for_each([&](auto pos, auto& cell) {
            auto _quad = [pos](float elev, auto& m) {
                auto x = pos.x, y = pos.y;
                plaza_geom::point corners[4] = {
                    {  x,   y, elev},
                    {x+1,   y, elev},
                    {x+1, y+1, elev},
                    {  x, y+1, elev}
                };
                return make_quad(corners[0], corners[1], corners[2], corners[3], m);
            };
            auto elev = normalize_int(cell);
            auto hd = _quad(elev, mesh);
            auto fd = face(hd, mesh);
            f2s[fd] = pos;

            if (elev < .5) {
                auto hdw = _quad(.5, eph);
                f2f[fd] = face(hdw, eph);
            }
        });

        auto i = add_mesh(mesh, to_rgba(palette.at("sand")));
        face_spaces[i] = f2s;

        auto ei = add_ephemeral(eph, to_rgba(palette.at("water"), .7));
        for (auto [f, ef]: f2f) {
            face_ephem[mesh_face(i, f)] = mesh_face(ei, ef);
        }
    };

    PZ_observe(ent_update) {
        auto render_grid = [&](auto pos, auto& skel)
        {
            auto elev = normalize_int(terrain.at(pos.r));
            skel.render(b_lines, fvec3(pos.r, elev) + fvec3(.5,.5,0));
        };
        auto render_free = [&](auto pos, auto& skel)
        {
            skel.render(b_lines, pos.r);
        };
        b_lines.clear();
        using namespace _cpt;
        entities.view<zpos, skel>().each(render_grid);
        entities.view<fpos, skel>().each(render_free);
        b_lines.update();
    };
}
