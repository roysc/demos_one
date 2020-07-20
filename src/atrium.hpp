#include "controls.hpp"
#include "map.hpp"
#include "input.hpp"
#include "spatial.hpp"
#include "geometry.hpp"
#include "index_mesh.hpp"
#include "util.hpp"

// #include "viewport.hpp"

#include <Rxt/graphics/shader/colored_triangle_3D.hpp>
#include <Rxt/graphics/shader/solid_color_3D.hpp>
#include <Rxt/graphics/sdl.hpp>
#include <Rxt/graphics/camera.hpp>
#include "reactive.hpp"

#include <map>
#include <optional>
#include <cstdint>

namespace plaza
{
using namespace plaza_geom;
}

// using std::chrono::steady_clock;
// using time_point = steady_clock::time_point;
namespace sdl = Rxt::sdl;
using Rxt::adapt_reactive_crt;
using Rxt::adapt_reactive;

// using panel_traits = spatial_traits<ivec, uvec>;
// using panel_viewport = basic_viewport<panel_traits>;
// using panel_layer = std::vector<std::pair<ivec, ivec>>; // todo index

struct reactive_toggle : Rxt::toggle_hooks
{
    bool state = false;
    reactive_toggle() {}
    operator bool() const { return state; }

    void disable()
    {
        state = false;
        this->on_disable();
    }

    void enable()
    {
        state = true;
        this->on_enable();
    }
};

template <class K, class V>
struct permissive_map
{
    using key_type = K;
    using value_type = V;
    std::map<K, V> _map;

    value_type& operator[](key_type k)
    {
        return get_or_emplace(_map, k, V{});
    }
};

using toggle_map = permissive_map<std::string, reactive_toggle>;

struct atrium_app : public sdl::simple_gui
{
    using triangle_program = Rxt::shader_programs::colored_triangle_3D;
    using line_program = Rxt::shader_programs::solid_color_3D<GL_LINES>;
    using point_program = Rxt::shader_programs::solid_color_3D<GL_POINTS>;

    struct cursor_traits
    {
        using position_type = fvec2;
        // using size_type = uvec2;
    };

    using cursor_type = adapt_reactive_crt<reactive_cursor, Rxt::hooks<>, cursor_traits>;

    using camera_state = Rxt::focused_camera;
    using camera_type = adapt_reactive_crt<reactive_cam, Rxt::hooks<>, camera_state>;
    struct drag_state { cursor_traits::position_type pos; camera_state cam; };

    bool quit = false;
    input_hooks input;
    sdl::key_dispatcher keys;
    uvec2 map_size;

    camera_state initial_camera;
    camera_type camera;
    cursor_type cursor;
    std::optional<drag_state> drag_origin;

    // panel_viewport ui_viewport;
    // panel_layer ui_objects;

    triangle_program triangle_prog;
    line_program line_prog, ui_line_prog;
    point_program point_prog;

    Rxt::hooks<> on_debug;

    atrium_app(const char*, uvec2);
    void advance(SDL_Event);
    bool is_stopped() const { return quit; }
    void draw();

    void _init_controls();
    void _init_ui();

    void handle_drag(fvec2, camera_state);

    virtual Rxt::reactive_handle model_updates() { return {}; }
};
