#include "map_editor.hpp"

#include <Rxt/range.hpp>
#include <Rxt/io.hpp>
#include <Rxt/graphics/color.hpp>
#include <Rxt/graphics/glm.hpp>

#include <string>
#include <iostream>

namespace gl = Rxt::gl;
namespace sdl = Rxt::sdl;
using Rxt::print;

extern "C" void step_state(void* c)
{
    sdl::step<map_editor>(c);
}

int main(int argc, char** argv)
{
    int seed = 42;
    if (argc > 1) {
        seed = std::stoi(argv[1]);
    }

    try {
        auto context = new map_editor(seed);
        auto loop = sdl::make_looper(context, step_state);
        loop();
    } catch (std::exception& e) {
        std::cout << "caught exception: " << e.what() << '\n';
    }

    return 0;
}

template <class F> void send(F&& f) { f(); } // todo wrapper for lazy updates

ivec nds_to_grid(glm::vec2 nds, glm::vec2 scale) { return floor(nds * scale); }

map_editor::map_editor(int seed)
    : grid_view{uvec{128}, uvec{4}}
    , texture_grid{static_cast<grid_view&>(*this)}
    , simple_gui{"Plaza: map_editor", viewport_size_px}
    , grid_layer{boost::extents[grid_size[0]][grid_size[1]]}
    , update_features{[this] { _update_features(); }}
    , update_cursor{[this] { _update_cursor(); }}
    , update_tool{[this] { _update_tool(); }}
    , update_viewport{[this] { _update_viewport(); }}
{
    keys.on_press["C-W"]    = [this] { should_quit(true); };
    keys.on_press["C"]      = [this] {
        viewport_position = ivec {0};
        send(update_viewport);
    };

    keys.on_press["Escape"] = [this] {
        current_tool = {};
        send(update_viewport);
        send(update_tool);
        print("Disabled modes\n");
    };

    keys.on_press["E"] = [this] { enable_edge_scroll = !enable_edge_scroll; };
    keys.on_press["S"] = [this] { current_tool = selection_tool {}; print("Tool: select\n"); };
    keys.on_press["B"] = [this] { current_tool = pen_tool {}; print("Tool: pen\n"); };

    keys.on_press["I"] = [this] {
        print("pos={} ", viewport_position);
        print("scale={} ", scale_factor);
        print("cursor={}\n", cursor_position);
        if (selection_tool select; get_tool(select) && select.selection) {
            auto [a, b] = *select.selection;
            print("selected: pos=({}, {}) size=({}, {})\n", a.x, a.y, b.x, b.y);
        }
    };

    auto move = [this] (int dx, int dy) {
        move_viewport(dx, dy);
        send(update_viewport);
    };
    auto scale = [this] (int a) {
        scale_viewport(a);
        send(update_viewport);
    };

    keys.on_scan["Left"]  = std::bind(move, -1, 0);
    keys.on_scan["Right"] = std::bind(move, +1, 0);
    keys.on_scan["Down"]  = std::bind(move, 0, -1);
    keys.on_scan["Up"]    = std::bind(move, 0, +1);
    keys.on_press["."] = std::bind(scale, +1);
    keys.on_press[","] = std::bind(scale, -1);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    auto image = create_map(grid_size, seed);
    update_texture(data(image));

    send(update_viewport);

    using Rxt::colors::russet;
    glClearColor(russet.r, russet.g, russet.b, 1);
}

void map_editor::step(SDL_Event event)
{
    do {
        handle(event);
    } while (SDL_PollEvent(&event));

    keys.scan();

    // Per-tick handlers
    if (enable_edge_scroll) {
        h_edge_scroll();
    }

    update_viewport.flush();
    update_features.flush();
    update_cursor.flush();
    update_tool.flush();

    if (is_dirty()) {
        draw();
        set_dirty(false);
    }
}

void map_editor::draw()
{
    glClear(GL_COLOR_BUFFER_BIT);

    texture_grid::draw();
    b_features.draw();
    b_quads.draw();

    { // todo: use UBOs and ubo_guard
        gl::uniform<ivec> u_vpos {quad_prog, "viewportPosition"};
        set(u_vpos, ivec {0});
        b_quads_sticky.draw();
        set(u_vpos, viewport_position);
    }

    SDL_GL_SwapWindow(window.get());
}

void map_editor::_update_features()
{
    b_features.clear();

    auto shape = grid_layer.shape();
    for (unsigned y = 0; y < shape[1]; ++y) {
        for (unsigned x = 0; x < shape[0]; ++x) {
            auto cell = grid_layer[x][y];
            if (cell != 0) {
                auto tile = tiles.at(cell);
                b_features.push(ivec{x, y}, uvec{1}, Rxt::rgba(tile.color, 1));
            }
        }
    }
    b_features.update();

    set_dirty();
}

void map_editor::_update_tool()
{
    // render selected regions
    b_quads.clear();
    if (selection_tool select; get_tool(select)) {
        for (auto [a, b]: Rxt::to_range(select.selection)) {
            b_quads.push(a, b, Rxt::rgba(Rxt::colors::hot_pink, 0.5)); //todo
        }
    }
    b_quads.update();

    set_dirty();
}

void map_editor::_update_cursor()
{
    ivec pos = cursor_position;
    uvec size{1};

    auto visitor = Rxt::overloaded {
        [&, this] (selection_tool& select) {
            // render cursor drag area
            if (auto& origin = select.drag_origin) {
                auto [a, b] = Rxt::ordered(*origin, cursor_position);
                pos = a;
                size = b - a + 1;
            }
        },
        [this] (pen_tool& pen) {
            if (pen.down) {
                ivec pos = cursor_position + viewport_position;
                pos %= grid_size;
                grid_layer[pos.x][pos.y] = pen.ink_fg;
                update_features();
            }
        },
        [] (auto) {}
    };
    visit(visitor, current_tool);

    b_quads_sticky.clear();
    b_quads_sticky.push(pos, size, cursor_color);
    b_quads_sticky.update();

    set_dirty();
}

void map_editor::h_mouse_motion(SDL_MouseMotionEvent motion)
{
    auto [x, y] = sdl::nds_coords(*window, motion.x, motion.y);
    cursor_position = nds_to_grid(glm::vec2{x, y}, glm::vec2(viewport_size() / 2u));
    update_cursor();
}

void map_editor::h_mouse_down(SDL_MouseButtonEvent button)
{
    auto left_visitor = Rxt::overloaded {
        [this] (selection_tool& select) {
            select.drag_origin = cursor_position;
            update_cursor();
        },
        [this] (pen_tool& pen) {
            pen.down = true;
            update_cursor();
        },
        [] (auto) {}
    };

    auto right_visitor = Rxt::overloaded {
        // Right-click while selecting grid cancels
        [this] (selection_tool& select) {
            if (select.drag_origin) {
                select.drag_origin.reset();
                update_cursor();
            } else if (select.selection) {
                select.selection = {};
                update_tool();
            }
        },
        [] (auto) {}
    };

    switch (button.button) {
    case SDL_BUTTON_LEFT: {
        visit(left_visitor, current_tool);
        break;
    }
    case SDL_BUTTON_RIGHT: {
        visit(right_visitor, current_tool);
        break;
    }
    }
}

void map_editor::h_mouse_up(SDL_MouseButtonEvent button)
{
    switch (button.button) {
    case SDL_BUTTON_LEFT: {
        auto visitor = Rxt::overloaded {
            [this] (selection_tool& select) {
                if (auto& origin = select.drag_origin) {
                    auto [a, b] = Rxt::ordered(*origin, cursor_position);
                    select.selection.emplace(a + viewport_position, b - a + 1);
                    select.drag_origin = {};
                    update_cursor();
                    update_tool();
                }
            },
            [this] (pen_tool& pen) {
                pen.down = false;
            },
            [] (auto) {}
        };
        visit(visitor, current_tool);
        break;
    }
    }
}

// edge-of-screen cursor scrolling
void map_editor::h_edge_scroll()
{
    // (0,0) is center-screen, so offset it to the corner
    uvec vpst = viewport_size();
    ivec offset_pos = cursor_position + ivec{vpst / 2u};
    ivec dv {0};
    for (unsigned i = 0; i < dv.length(); ++i) {
        if (offset_pos[i] == 0) {
            dv[i] = -1;
        } else if (offset_pos[i] + 1 == vpst[i]) {
            dv[i] = +1;
        }
    }
    if (dv != ivec{0}) {
        move_viewport(dv.x, dv.y);
    }
}

void map_editor::handle(SDL_Event event)
{
    switch (event.type) {
    case SDL_QUIT: { should_quit(true); return; }
    case SDL_KEYDOWN: { keys.press(event.key.keysym); break; }
    case SDL_MOUSEMOTION: { h_mouse_motion(event.motion); break; }
    case SDL_MOUSEBUTTONDOWN: { h_mouse_down(event.button); break; }
    case SDL_MOUSEBUTTONUP: { h_mouse_up(event.button); break; }
    }
}

void map_editor::_update_viewport()
{
    texture_grid::update_viewport();

    gl::set_uniform(quad_prog, "viewportPosition", viewport_position);
    set_uniform(quad_prog, "viewportSize", viewport_size());

    set_dirty();
}
