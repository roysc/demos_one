#pragma once

#include <Rxt/graphics/gl.hpp>
#include <Rxt/graphics/shader/texture_quad_2D.hpp>
#include <Rxt/util.hpp>

#include "grid_view.hpp"

struct texture_grid : public virtual grid_view
{
    using texture_program = Rxt::shader_programs::texture_quad_2D;

    uvec grid_size;
    texture_program tex_prog{};
    texture_program::data b_texture {tex_prog};

    texture_grid(grid_view);
    texture_program& texture_prog() { return tex_prog; }

    virtual void update_texture(void*);
    virtual void update_viewport();

    void draw() { b_texture.draw(); }
};
