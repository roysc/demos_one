#include "plant.hpp"
#include "noise.hpp"
#include <Rxt/math.hpp>

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

    opts["highlight_face"].enable();
    // opts["highlight_vertex"].enable();
}

void plant_app::draw_clear()
{
    auto bg = palette.at("bg");
    glClearColor(bg.r, bg.g, bg.b, 1);
    // super_type::draw();
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

mesh_key plant_app::put_mesh(mesh_type mesh, mesh_color color, entity_id ent)
{
    auto ix = geom.insert(mesh);
    geom.build();

    if (!entities.valid(ent))
        ent = entities.create();
    entities.emplace<cpt::mesh>(ent, &geom, ix, color);
    return ix;
}

mesh_key plant_app::put_ephemeral(mesh_type mesh, mesh_color color, entity_id ent)
{
    auto ix = ephem.insert(mesh);
    ephem.build();

    if (!entities.valid(ent))
        ent = entities.create();
    entities.emplace<cpt::mesh>(ent, &ephem, ix, color, true);
    return ix;
}
