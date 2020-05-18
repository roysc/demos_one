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

    uvec size{64};
    try {
        auto loop = sdl::make_looper(new map_editor(seed, size), step_state);
        loop();
    } catch (std::exception& e) {
        std::cout << "caught exception: " << e.what() << '\n';
    }

    return 0;
}

template <class F> void send(F&& f) { f(); } // todo wrapper for lazy updates/signal

ivec nds_to_grid(glm::vec2 nds, glm::vec2 scale) { return floor(nds * scale); }

map_editor::map_editor(int seed, uvec size, grid_viewport vp)
    : simple_gui{"plaza: map_editor", vp.size_pixels()} // fixme
    , grid_size{size}
    , viewport{vp}
    , background{size}
    , grid_layer{boost::extents[viewport.max_scale[0]][viewport.max_scale[1]]}
    , update_features{[this] { _update_features(); }}
    , update_cursor{[this] { _update_cursor(); }}
    , update_tool{[this] { _update_tool(); }}
    , update_viewport{
    [this] {
        interface.set_viewport(viewport);
        background.set_viewport(viewport);
        set_dirty();
    }}
{
    keys.on_press["C-W"]    = [this] { _quit = true; };
    keys.on_press["C"]      = [this] {
        viewport.position(ivec{0});
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
                             print("pos={} ", viewport.position());
        print("scale={} ", viewport.scale_factor);
        print("cursor={}\n", cursor_position);
        if (selection_tool select; get_tool(select) && select.selection) {
            auto [a, b] = *select.selection;
            print("selected: pos=({}, {}) size=({}, {})\n", a.x, a.y, b.x, b.y);
        }
    };

    auto move = [this] (int dx, int dy) {
        viewport.move(dx, dy);
        send(update_viewport);
    };
    auto scale = [this] (int a) {
        viewport.scale(a);
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
    background.set_texture(data(image));

    send(update_viewport);

    using Rxt::colors::russet;
    glClearColor(russet.r, russet.g, russet.b, 1);
}

void map_editor::step(SDL_Event event)
{
    do {
        handle_input(event);
    } while (SDL_PollEvent(&event));

    keys.scan();

    // Per-tick handlers
    if (enable_edge_scroll) {
        viewport.edge_scroll(cursor_position, 1);
        send(update_viewport);
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

    background.draw();
    b_features.draw();
    interface.draw(viewport);

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
    if (selection_tool select; get_tool(select)) {
        for (auto [a, b]: Rxt::to_range(select.selection)) {
            interface.set_selection(a, b);
        }
    }
}

void map_editor::_update_cursor()
{
    ivec pos = cursor_position;
    uvec size{1};

    auto visitor = Rxt::overloaded {
        [&, this] (selection_tool& select) {
            // render cursor drag area
            if (auto& origin = select.drag_origin) {
                auto [a, b] = Rxt::box(*origin, cursor_position);
                pos = a;
                size = b - a + 1;
            }
        },
        [this] (pen_tool& pen) {
            if (pen.down) {
                ivec penpos = cursor_position + viewport.position();
                penpos %= grid_size;
                grid_layer[penpos.x][penpos.y] = pen.ink_fg;
                send(update_features);
            }
        },
        [] (auto) {}
    };
    visit(visitor, current_tool);
    interface.set_cursor(pos, size);

    set_dirty();
}

void map_editor::handle_mouse_motion(SDL_MouseMotionEvent motion)
{
    auto [x, y] = sdl::nds_coords(*window, motion.x, motion.y);
    cursor_position = nds_to_grid(glm::vec2(x, y), glm::vec2(viewport.size_cells() / 2u));
    send(update_cursor);
}

void map_editor::handle_mouse_down(SDL_MouseButtonEvent button)
{
    auto left_visitor = Rxt::overloaded {
        [this] (selection_tool& select) {
            select.drag_origin = cursor_position;
            send(update_cursor);
        },
        [this] (pen_tool& pen) {
            pen.down = true;
            send(update_cursor);
        },
        [] (auto) {}
    };

    auto right_visitor = Rxt::overloaded {
        // Right-click while selecting grid cancels
        [this] (selection_tool& select) {
            if (select.drag_origin) {
                select.drag_origin.reset();
                send(update_cursor);
            } else if (select.selection) {
                select.selection = {};
                send(update_tool);
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

void map_editor::handle_mouse_up(SDL_MouseButtonEvent button)
{
    switch (button.button) {
    case SDL_BUTTON_LEFT: {
        auto visitor = Rxt::overloaded {
            [this] (selection_tool& select) {
                if (auto& origin = select.drag_origin) {
                    auto [a, b] = Rxt::box(*origin, cursor_position);
                    select.selection.emplace(a + viewport.position(), b - a + 1);
                    select.drag_origin = {};
                    send(update_cursor);
                    send(update_tool);
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
