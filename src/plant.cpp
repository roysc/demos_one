#include "plant.hpp"
#include "rendering.hpp"
#include "noise.hpp"

#include <Rxt/graphics/color.hpp>

using Rxt::to_rgba;
using Rxt::print;

template <class I>
static float normalize_int(I t)
{
    I max_elev = std::numeric_limits<I>::max();
    return float(t)/max_elev;
}

plant_app::plant_app(uvec2 size)
    : super_type("plaza: plant", size)
    , palette(default_palette())
{
    _init_ui();
    _init_model();

    terrain_map tm(map_size);
    auto scale = 0xFF;
    fill_noise(map_size, 42, [&](int x, int y, auto a) { tm.put({x, y}, a * scale / 2); });
    terrain.emplace(tm);

    e_debug = entities.create();

    PZ_observe(input.on_mouse_down, SDL_MouseButtonEvent button) {
        auto paint = [this](ivec2 pos)
        {
            auto plant = build_plant();
            auto id = (std::size_t)put_entity(entities, pos, plant);
            _ent_update();
            print("put({}): plant({})\n", pos, id);
        };

        switch (button.button) {
        case SDL_BUTTON_LEFT: {
            if (auto pos = highlighted_space()) {
                paint(*pos);
            }
            break;
        }
        }
    };

    opts["highlight_face"].enable();
    // opts["highlight_vertex"].enable();
}

void plant_app::draw_clear()
{
    auto bg = palette.at("bg");
    glClearColor(bg.r, bg.g, bg.b, 1);
    // super_type::draw();
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
}

void plant_app::_init_model()
{
    auto& b_triangles = triangle_prog.buf["triangles"];
    auto& b_tris_txp = triangle_prog.buf["tris_txp"];
    auto& b_lines = line_prog.buf["lines"];
    auto& b_overlines = line_prog.buf["overlines"];

    PZ_observe(_model_update) {
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

    PZ_observe(_ent_update) {
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

void plant_app::advance(SDL_Event event)
{
    auto up1 = super_type::_update(event);
    auto up2 = {
        &_model_update,
        &highlighted_faces.on_update,
    };
    auto dirty = flush_all(up1) + flush_all(up2);
    if (dirty) {
        draw_clear();
        super_type::draw();
        // draw();
    }
}

std::optional<ivec2> plant_app::highlighted_space() const
{
    if (highlighted_faces) {
        auto [oi, fd] = *highlighted_faces;
        ivec2 pos = face_spaces.at(oi).at(fd);
        assert(Rxt::point_within(pos, terrain.shape()));
        return pos;
    }
    return std::nullopt;
}

mesh_key plant_app::add_mesh(mesh3 mesh, Rxt::rgba color)
{
    auto ix = geom.insert(mesh);
    geom.build();
    colors.emplace(ix, color);
    _model_update();
    return ix;
}

mesh_key plant_app::add_ephemeral(mesh3 mesh, Rxt::rgba color)
{
    auto ix = ephem.insert(mesh);
    ephem.build();
    ephem_colors.emplace(ix, color);
    _model_update();
    return ix;
}
