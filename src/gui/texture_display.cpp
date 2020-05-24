#include "texture_display.hpp"
#include "../util.hpp"

#include <glm/gtx/transform.hpp>

namespace gl = Rxt::gl;

texture_display::texture_display(uvec size)
    : grid_size{size}
{}

void texture_display::set_texture(void* image)
{
    gl::use_guard _g(tex_prog);
    gl::bind_vao_guard _a(b_texture.va);
    glActiveTexture(GL_TEXTURE0);
    gl::bind_texture_guard _t(GL_TEXTURE_2D, b_texture.tex);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); // or GL_CLAMP_TO_BORDER
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, grid_size.x, grid_size.y, 0, GL_RGBA,
                 GL_UNSIGNED_BYTE, image);

    b_texture.position.storage = {{-1, -1}, {1, -1}, {-1, 1}, {1, 1}};
    b_texture.tex_coord.storage = {{0, 0}, {1, 0}, {0, 1}, {1, 1}};
    b_texture.elements.storage = {0, 1, 2, 2, 3, 0};
    b_texture.update();
}

void texture_display::set_viewport(grid_viewport const& viewport)
{
    using glm::vec2;
    using glm::vec3;

    vec2 vp_rel_size = vec2(viewport.size_cells()) / vec2(grid_size);
    vec2 vp_rel_pos = vec2(viewport.position()) / vec2(grid_size);
    glm::mat4 tex_view_matrix =
        glm::translate(vec3(vp_rel_pos, 0)) *
        glm::scale(vec3(invert<1>(vp_rel_size), 0)) *
        glm::translate(vec3(-.5, -.5, 0));

    gl::set_uniform(tex_prog, "viewMatrix", tex_view_matrix);
}
