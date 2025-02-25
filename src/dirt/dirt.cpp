#include "dirt.hpp"
#include <Rxt/math.hpp>
#include <Rxt/loop.hpp>

#include <string>

dirt_app::dirt_app(viewport_size_type size)
    : super_type("atrium: dirt", size)
    , palette(default_palette())
    , space(Rxt::vec::uvec2(8), 42)
{
    initial_camera = default_camera(space.stage_size());

    _init_ui();
    _init_model();
    // e_debug = entities.create();
    opts["highlight_face"].enable();
    // opts["highlight_vertex"].enable();

    active_stage.emplace(space.root());
    reset_camera();
    // auto unitbounds = Rxt::box(fvec2(-.5), fvec2(.5));
    // auto normbounds = Rxt::box(fvec2(-1), fvec2(1));
    // stage_cache.emplace_back(entities, normbounds);
    // active_stage = &stage_cache[0];

    // dirt stage
    // auto e1 = put_entity(entities, ivec2(0), board_unit);
    // entities.emplace<mesh>(e1, make_thing(geom, e1));
    // put_mesh(thing_mesh(), idk, &e1);
    // active_stage->add(e1);

    // set active stage
    // allow to zoom to substages
    // stage size / zoom factor ctc?
    // stationary objects in quadtree
    // + walls
    // mobile objects in hash?

    // https://ourmachinery.com/post/syncing-a-data-oriented-ecs/
    // changing<component> for update?

    auto zoom = [&] {
        cell_position pos;
        if (!highlighted_space(pos))
            return;
        auto substage = active_stage->get_substage(pos);
        Rxt::print("zooming to {}\n", substage->get_path());
        active_stage.emplace(substage);
    };
    keys().on_press["F"] = zoom;
}

int main(int argc, char* argv[])
{
    int seed = 42;
    if (argc > 1) {
        seed = std::stoi(argv[1]);
    }

    auto vpsize = dirt_app::viewport_size_type{800};
    auto context = new dirt_app(vpsize);
    Rxt::sdl::run_loop(context);

    return 0;
}

void dirt_app::advance(Rxt::loop_duration)
{
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        input.handle_input(event);
    }
    keys().scan();

    auto updates = Rxt::make_hooks(
        super_type::_updates(event),
        _model_update,
        highlighted_faces.on_update
    );
    updates.flush();

    auto bg = palette.at("bg");
    glClearColor(bg.r, bg.g, bg.b, 1);
    super_type::draw();
}

bool dirt_app::highlighted_space(cell_position& out) const
{
    if (highlighted_faces) {
        auto [oi, fd] = *highlighted_faces;
        auto pos = face_spaces.at(oi).at(fd);
        assert(Rxt::point_within(pos, active_stage->size()));
        out = pos;
        return true;
    }
    return false;
}

entity_id dirt_app::put_mesh(mesh_type mesh, mesh_color color, bool transparent, mesh_kind kind)
{
    auto* meshes = _geom[kind];
    auto ix = meshes->insert(mesh);
    meshes->build();

    auto ent = entities.create();
    entities.emplace<cpt::mesh>(ent, meshes, ix, color, transparent);
    return ent;
}
