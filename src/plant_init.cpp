#include "plant.hpp"
#include "rendering.hpp"
#include <Rxt/color.hpp>
#include <Rxt/vec.hpp>
#include <Rxt/vec_io.hpp>

using Rxt::print;
using Rxt::to_rgba;

template <class I>
static float normalize_int(I t)
{
    I max_elev = std::numeric_limits<I>::max();
    return float(t)/max_elev;
}

void plant_app::_init_ui()
{
    PZ_observe(on_debug) {
        print("camera.pos={} .focus={} .up={}\n", camera.position(), camera.focus, camera.up);
        if (drag_origin) print("drag_origin = {}\n", drag_origin->pos);
    };

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
    };

    PZ_observe(input.on_mouse_down, SDL_MouseButtonEvent button) {
        auto paint = [this](position_ivec pos)
        {
            if (!active_stage) {
                print("error: No active stage\n");
                return;
            }

            auto thing = planty::build_house();
            auto ent = put_mesh(thing, to_rgba(Rxt::colors::gray));
            entities.emplace<cpt::cell>(ent, *active_stage, pos);
            entities.emplace<cpt::nam>(ent, "house");

            _model_update();
            print("put({}): {}({})\n", pos, entity_name(entities, ent), (std::size_t)ent);
        };

        switch (button.button) {
        case SDL_BUTTON_LEFT: {
            if (position_ivec pos; highlighted_space(pos)) {
                paint(pos);
            }
            break;
        }
        }
    };
}

void plant_app::load_stage(stage_type& stage)
{
    using Sfd = mesh_data::source_face_descriptor;
    mesh_type mesh, eph;
    face_to_space f2s;
    std::map<Sfd, Sfd> f2f;

    stage.grid().for_each([&](auto pos, auto& cell) {
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

    auto entity_mesh = [&] (auto e) { return entities.get<cpt::mesh>(e); };

    auto ent = put_mesh(mesh, palette.at("sand"));
    auto meshid = entity_mesh(ent).key;
    auto ephent = put_mesh(eph, to_rgba(palette.at("water"), .7), true);
    auto ephid = entity_mesh(ephent).key;
    set_parent_entity(entities, ent, ephent);

    // map tangible face to stage position
    face_spaces[meshid] = f2s;
    // map each tangible face to a child's ephemeral face
    for (auto [mf, ef]: f2f) {
        face_ephem[mesh_face(meshid, mf)] = mesh_face(ephid, ef);
    }
    _model_update();
}

void plant_app::_init_model()
{
    auto& b_triangles = triangle_prog.buf["triangles"];
    auto& b_tris_txp = triangle_prog.buf["tris_txp"];
    auto& b_lines = line_prog.buf["lines"];
    auto& b_overlines = line_prog.buf["overlines"];

    PZ_observe(active_stage.on_update, auto stage) {

        load_stage(*stage);
    };

    PZ_observe(_model_update) {
        auto free_mesh = [&] (auto& g, cpt::fpos pos)
        {
            auto tm = Rxt::translate(pos.r);
            g.render(g.transparent ? b_tris_txp : b_triangles, tm);
        };
        auto cell_mesh = [&] (cpt::cell cell, auto& g)
        {
            g.render(
                g.transparent ? b_tris_txp : b_triangles,
                Rxt::translate(cell.offset<position_fvec>())
            );
        };

        b_triangles.clear();
        b_tris_txp.clear();
        entities.view<cpt::mesh>().each([&] (auto& geom) { free_mesh(geom, cpt::fpos3()); });
        // entities.view<cpt::fpos, cpt::mesh>().each(free_mesh);
        entities.view<cpt::cell, cpt::mesh>().each(cell_mesh);
        b_triangles.update();
        b_tris_txp.update();

        auto cell_skel = [&](auto cell, auto& g)
        {
            g.render(b_lines, Rxt::translate(cell.offset<position_fvec>()));
        };
        b_lines.clear();
        entities.view<cpt::cell, cpt::skel>().each(cell_skel);
        b_lines.update();
    };

    PZ_observe(highlighted_faces.on_update) {
        auto& b_overlines = line_prog.buf["over_lines_hl"];
        b_overlines.clear();
        if (highlighted_faces) {
            render_hl(*highlighted_faces, geom, b_overlines, palette.at("hl"));
            if (auto others = face_ephem.find(*highlighted_faces); others != end(face_ephem))
                render_hl(others->second, ephem, b_overlines, palette.at("hl_water"));
        }
        b_overlines.update();
    };

    PZ_observe(highlighted_vertices.on_update) {
        auto& b = point_prog.buf["points"];
        b.clear();
        for (auto [oi, vd]: highlighted_vertices) {
            auto pos = get(CGAL::vertex_point, geom.sources[oi], vd);
            b.push(to_glm(pos), Rxt::colors::white);
        }
        b.update();
    };
}
