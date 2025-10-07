#include "dirt.hpp"
#include "dirt/dirt_models.hpp"
#include "rendering.hpp"
#include "stage.hpp"

#include <Rxt/color.hpp>
#include <Rxt/vec.hpp>
#include <CGAL/boost/graph/generators.h>

#include <print>

using std::print;

template <class I>
static float normalize_int(I t)
{
    I max_elev = std::numeric_limits<I>::max();
    return float(t)/max_elev;
}

void dirt_app::_init_ui()
{
    on_debug += [this] {
        print("camera.pos={} .focus={} .up={}\n", camera.position(), camera.focus, camera.up);
        if (drag_origin) print("drag_origin = {}\n", drag_origin->pos);
    };

    opts["highlight_face"].on_disable += [this] {
        highlighted_faces.emplace();
        highlighted_faces.on_update();
    };
    opts["highlight_vertex"].on_disable += [this] {
        highlighted_vertices.clear();
        highlighted_vertices.on_update();
    };

    cursor.on_update += [this] {
        using namespace geom;
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

    auto paint = [this] (auto factory)
    {
        cell_position pos;
        if (!highlighted_space(pos)) return;
        if (!active_stage) {
            print("error: No active stage\n");
            return;
        }

        auto thing = factory();
        auto ent = put_mesh(thing, Rxt::colors::gray, false);
        entities.emplace<z2_cell>(ent, *active_stage, pos);
        _model_update();
        print("put({}): ({})\n", pos, static_cast<std::size_t>(ent));
        // entities.emplace<cpt::name>(ent, "house");
        // entities.emplace<cpt::input<cpt::name>>(ent);
        // print("put({}): {}({})\n", pos, entity_name(entities, ent), (std::size_t)ent);
    };

    input.on_mouse_down += [=] (SDL_MouseButtonEvent button) {
        switch (button.button) {
        case SDL_BUTTON_LEFT: {
            paint(&build_house);
        }
        }
    };

    keys().on_press["1"] = std::bind(paint, &build_house);
    keys().on_press["2"] = std::bind(paint, &build_tetroid);
    keys().on_press["T"] = [this] {
        _tick++;
        _ent_update();
        // what does it mean?
    };
}

// load stage with already transformed position
entity_id dirt_app::update_stage(stage_type& stage)
{
    using Sfd = mesh_index::source_face_descriptor;
    mesh_type mesh, eph;
    face_to_space f2s;
    std::map<Sfd, Sfd> f2f;

    stage.grid().for_each([&](auto pos, auto& cell) {
        auto _quad = [pos](float elev, auto& m) {
            auto x = pos.x, y = pos.y;
            geom::point corners[4] = {
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

    auto entity_mesh = [&] (auto e) { return entities.get<mesh_geom>(e); };

    auto ent = put_mesh(mesh, palette.at("sand"), false);
    auto meshid = entity_mesh(ent).key;
    auto ephent = put_mesh(eph, palette.at("water"), .7, true, mesh_kind::ephemeral);
    auto ephid = entity_mesh(ephent).key;
    set_parent_entity(entities, ent, ephent);
    entities.emplace<fpos3>(ent, Rxt::vec::fvec3(0));
    entities.emplace<fpos3>(ephent, Rxt::vec::fvec3(0));

    // map tangible face to stage position
    face_spaces[meshid] = f2s;
    // map each tangible face to a child's ephemeral face
    for (auto [mf, ef]: f2f) {
        face_ephem[mesh_face(meshid, mf)] = mesh_face(ephid, ef);
    }
    _model_update();
    return ent;
}

void dirt_app::_init_model()
{
    auto& b_triangles = triangle_prog.buf("triangles");
    auto& b_transparent_tris = triangle_prog.buf("transparent_tris");
    auto& b_lines = line_prog.buf("lines");
    // auto& b_overlines = line_prog.buf("overlines");

    active_stage.on_update += [this] (auto stage) {
        this->update_stage(*stage);
    };

    _model_update += [&, this] {
        auto free_mesh = [&] (fpos3 pos, auto& g)
        {
            auto tm = translate(pos.r);
            g.render(g.transparent ? b_transparent_tris : b_triangles, tm);
        };
        auto cell_mesh = [&] (z2_cell cell, auto& g)
        {
            auto tm = translate(offset<free_position>(cell));
            g.render(g.transparent ? b_transparent_tris : b_triangles, tm);
        };
        auto cell_skel = [&](auto cell, auto& g)
        {
            auto tm = translate(offset<free_position>(cell));
            g.render(b_lines, tm);
        };

        b_triangles.clear();
        b_transparent_tris.clear();
        // entities.view<cpt::mesh>().each([&] (auto& m) { free_mesh(m, cpt::fpos3()); });
        entities.view<fpos3, mesh_geom>().each(free_mesh);
        entities.view<z2_cell, mesh_geom>().each(cell_mesh);
        b_triangles.update();
        b_transparent_tris.update();
        b_lines.clear();
        entities.view<z2_cell, skel_geom>().each(cell_skel);
        b_lines.update();
    };

    highlighted_faces.on_update += [this] {
        auto& buf = line_prog.buf("over_lines_hl");
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

    highlighted_vertices.on_update += [this] {
        auto& b = point_prog.buf("points");
        b.clear();
        for (auto [oi, vd]: highlighted_vertices) {
            auto pos = get(CGAL::vertex_point, _mesh_index().sources[oi], vd);
            b.push(to_glm(pos), Rxt::colors::white);
        }
        b.update();
    };
}
