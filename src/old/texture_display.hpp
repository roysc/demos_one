#pragma once

#include "grid_viewport.hpp"

#include <Rxt/graphics/gl.hpp>
#include <Rxt/graphics/shader/texture_quad_2D.hpp>
#include <Rxt/util.hpp>

struct texture_display
{
    using texture_program = Rxt::shader_programs::texture_quad_2D;

    uvec grid_size;
    Rxt::gl::texture tex;
    texture_program tex_prog;
    texture_program::buffers b_texture {tex_prog};

    texture_display(uvec);
    texture_program& texture_prog() { return tex_prog; }

    void set_texture(void*);
    void set_viewport(grid_viewport const&);

    void draw() { b_texture.draw(); }
};
