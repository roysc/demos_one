#pragma once

#include <Rxt/runtime.hpp>
#include <Rxt/graphics/sdl.hpp>

#ifdef RXT_WEBGL2
  #include <Rxt/graphics/shader/webgl_grid_quad_2D.hpp>
#else
  #include <Rxt/graphics/shader/grid_quad_2D.hpp>
#endif

struct interface_display
{
#ifdef RXT_WEBGL2
    using grid_program = Rxt::shader_programs::webgl::grid_quad_2D;
#else
    using grid_program = Rxt::shader_programs::grid_quad_2D;
#endif

    const Rxt::rgba cursor_color {0, 1, 1, .5};
    const Rxt::rgba selection_color {Rxt::colors::hot_pink, 0.5};

    grid_program quad_prog;
    grid_program::data b_quads {quad_prog};
    grid_program::data b_quads_sticky {quad_prog}; // for cursor

    interface_display() {}

    void set_viewport(grid_viewport const& v)
    {
        set_uniform(quad_prog, "viewportPosition", v.position);
        set_uniform(quad_prog, "viewportSize", v.size_cells());
    }

    void set_cursor(ivec pos, uvec size)
    {
        b_quads_sticky.clear();
        b_quads_sticky.push(pos, size, cursor_color);
        b_quads_sticky.update();
    }

    void set_selection(ivec pos, uvec size)
    {
        b_quads.clear();
        b_quads.push(pos, size, selection_color);
        b_quads.update();
    }

    void draw(grid_viewport const& viewport)
    {
        b_quads.draw();

        { // todo: use UBOs and ubo_guard
            Rxt::gl::uniform<ivec> u_vpos {quad_prog, "viewportPosition"};
            set(u_vpos, ivec {0});
            b_quads_sticky.draw();
            set(u_vpos, viewport.position);
        }
    }
};
