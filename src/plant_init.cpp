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
        using namespace atrium_geom;
        auto [source, dir] = Rxt::cast_ray(cursor.position(), camera);
        auto& geom = _mesh_index();

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

    auto paint = [this] (auto&& creator)
    {
        cell_position pos;
        if (!highlighted_space(pos)) return;

        if (!active_stage) {
            print("error: No active stage\n");
            return;
        }

        auto thing = creator();
        auto ent = put_mesh(thing, to_rgba(Rxt::colors::gray), false);
        entities.emplace<cpt::cell>(ent, *active_stage, pos);
        _model_update();
        print("put({}): ({})\n", pos, (std::size_t)ent);
        // entities.emplace<cpt::name>(ent, "house");
        // print("put({}): {}({})\n", pos, entity_name(entities, ent), (std::size_t)ent);
    };

    PZ_observe(input.on_mouse_down, SDL_MouseButtonEvent button) {
        switch (button.button) {
        case SDL_BUTTON_LEFT: {
            paint(&planty::build_house);
        }
        }
    };

    keys().on_press["1"] = std::bind(paint, &planty::build_house);
    keys().on_press["2"] = std::bind(paint, &planty::build_tetroid);
}

// load stage with already transformed position
entity_id plant_app::load_stage(stage_type& stage)
{
    using Sfd = mesh_index::source_face_descriptor;
    mesh_type mesh, eph;
    face_to_space f2s;
    std::map<Sfd, Sfd> f2f;

    stage.grid().for_each([&](auto pos, auto& cell) {
        auto _quad = [pos](float elev, auto& m) {
            auto x = pos.x, y = pos.y;
            atrium_geom::point corners[4] = {
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

    auto ent = put_mesh(mesh, palette.at("sand"), false);
    auto meshid = entity_mesh(ent).key;
    auto ephent = put_mesh(eph, to_rgba(palette.at("water"), .7), true, mesh_kind::ephemeral);
    auto ephid = entity_mesh(ephent).key;
    set_parent_entity(entities, ent, ephent);
    entities.emplace<cpt::fpos3>(ent, Rxt::fvec3(0));
    entities.emplace<cpt::fpos3>(ephent, Rxt::fvec3(0));

    // map tangible face to stage position
    face_spaces[meshid] = f2s;
    // map each tangible face to a child's ephemeral face
    for (auto [mf, ef]: f2f) {
        face_ephem[mesh_face(meshid, mf)] = mesh_face(ephid, ef);
    }
    _model_update();
    return ent;
}

void plant_app::_init_model()
{
    auto& b_triangles = triangle_prog.buf["triangles"];
    auto& b_tris_txp = triangle_prog.buf["tris_txp"];
    auto& b_lines = line_prog.buf["lines"];
    auto& b_overlines = line_prog.buf["overlines"];

    PZ_observe(active_stage.on_update, auto stage) {
        this->load_stage(*stage);
    };

    PZ_observe(_model_update) {
        auto free_mesh = [&] (cpt::fpos3 pos, auto& g)
        {
            auto tm = Rxt::translate(pos.r);
            g.render(g.transparent ? b_tris_txp : b_triangles, tm);
        };
        auto cell_mesh = [&] (cpt::cell cell, auto& g)
        {
            auto tm = Rxt::translate(cell.offset<free_position>());
            g.render(g.transparent ? b_tris_txp : b_triangles, tm);
        };
        auto cell_skel = [&](auto cell, auto& g)
        {
            auto tm = Rxt::translate(cell.template offset<free_position>());
            g.render(b_lines, tm);
        };

        b_triangles.clear();
        b_tris_txp.clear();
        // entities.view<cpt::mesh>().each([&] (auto& m) { free_mesh(m, cpt::fpos3()); });
        entities.view<cpt::fpos3, cpt::mesh>().each(free_mesh);
        entities.view<cpt::cell, cpt::mesh>().each(cell_mesh);
        b_triangles.update();
        b_tris_txp.update();
        b_lines.clear();
        entities.view<cpt::cell, cpt::skel>().each(cell_skel);
        b_lines.update();
    };

    PZ_observe(highlighted_faces.on_update) {
        auto& buf = line_prog.buf["over_lines_hl"];
        buf.clear();
        if (highlighted_faces) {
            render_hl(*highlighted_faces, _mesh_index(),
                      buf, palette.at("hl"));
            if (auto others = face_ephem.find(*highlighted_faces); others != end(face_ephem))
                render_hl(others->second, _mesh_index(mesh_kind::ephemeral),
                          buf, palette.at("hl_water"));
        }
        buf.update();
    };

    PZ_observe(highlighted_vertices.on_update) {
        auto& b = point_prog.buf["points"];
        b.clear();
        for (auto [oi, vd]: highlighted_vertices) {
            auto pos = get(CGAL::vertex_point, _mesh_index().sources[oi], vd);
            b.push(to_glm(pos), Rxt::colors::white);
        }
        b.update();
    };
}
