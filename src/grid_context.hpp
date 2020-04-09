#pragma once

#include "space.hpp"

#include <Rxt/graphics/sdl.hpp>
#include <Rxt/graphics/gl.hpp>
#include <Rxt/graphics/shader/texture_quad_2D.hpp>
#include <Rxt/graphics/shader/grid_quad_2D.hpp>

#include <Rxt/util.hpp>
#include <Rxt/range.hpp>

#include <glm/gtx/transform.hpp>

namespace gl = Rxt::gl;
using Rxt::shader_programs::texture_quad_2D;
using Rxt::shader_programs::grid_quad_2D;

template <class V>
auto invert_y(V v) { v.y = -v.y; return v; }

struct grid_context : public Rxt::sdl::simple_gui
{
    gl::program_loader loader;

    grid_size world_size;
    grid_size viewport_size;
    glm::uvec2 tile_size_px;

    grid_coord viewport_position {0};

    texture_quad_2D tex_prog {loader};
    grid_quad_2D quad_prog {loader};

    texture_quad_2D::data b_texs {tex_prog};
    grid_quad_2D::data b_quads {quad_prog};

    grid_context(const char* title, grid_size world_size, glm::uvec2 tile_px);

    static constexpr bool is_torus() { return true; }

    void update_model();
    void update_texture(void*);

    virtual void update_viewport();

    void h_move_viewport(int dx, int dy)
    {
        viewport_position += grid_coord {dx, dy};
        if constexpr (is_torus()) {
            viewport_position %= grid_coord(world_size);
        }
        update_viewport();
    }

    void h_scale_viewport(int);
};
