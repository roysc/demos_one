#pragma once

#include <Rxt/graphics/sdl.hpp>
#include <Rxt/graphics/gl.hpp>
#include <Rxt/graphics/shader/texture_quad_2D.hpp>
#ifdef RXT_WEBGL2
  #include <Rxt/graphics/shader/webgl_grid_quad_2D.hpp>
#else
  #include <Rxt/graphics/shader/grid_quad_2D.hpp>
#endif
#include <Rxt/util.hpp>

using grid_coord = glm::ivec2;
using grid_offset = glm::ivec2;
using grid_size = glm::uvec2;

struct grid_display : public Rxt::sdl::simple_gui
{
    using texture_program = Rxt::shader_programs::texture_quad_2D;
#ifdef RXT_WEBGL2
    using grid_program = Rxt::shader_programs::webgl::grid_quad_2D;
#else
    using grid_program = Rxt::shader_programs::grid_quad_2D;
#endif
    Rxt::gl::program_loader loader;

    grid_size world_size;
    grid_size viewport_size;
    grid_size tile_size_px;

    grid_coord _viewport_position {0};

    texture_program tex_prog {loader};
    grid_program quad_prog {loader};

    texture_program::data b_background {tex_prog};

    Rxt::lazy_action update_model, update_viewport;

    grid_display(char const* title, grid_size world_size, grid_size tile_px);
    ~grid_display() override {}

    void _update_background(void*);
    void _update_model();
    void _update_viewport();

    void move_viewport(int dx, int dy)
    {
        _viewport_position += grid_offset{dx, dy};
        update_viewport();
    }

    void scale_viewport(int);
};
