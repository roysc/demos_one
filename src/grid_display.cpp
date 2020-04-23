#include "grid_display.hpp"

#include <glm/gtx/transform.hpp>

namespace gl = Rxt::gl;

template <unsigned ix, class Vec>
Vec invert(Vec v) { v[ix] = -v[ix]; return v; }

grid_display::grid_display(const char* title, grid_size world_size, grid_size tile_px)
    : simple_gui(title, tile_px * world_size)
    , world_size(world_size)
    , viewport_size(world_size)
    , tile_size_px(tile_px)
    , update_model{[this] { _update_model(); }}
    , update_viewport{[this] { _update_viewport(); }}
{}

void grid_display::scale_viewport(int exp)
{
    // simulate zoom in/out by scaling down/up resp.; correct position to keep centered
    if (exp > 0) {
        if (viewport_size.x >= world_size.x || viewport_size.y >= world_size.y)
            return;
        viewport_size *= 2;
        tile_size_px /= 2;
    } else {
        if (viewport_size.x <= 1 || viewport_size.y <= 1)
            return;
        viewport_size /= 2;
        tile_size_px *= 2;
    }
    update_viewport();
}

void grid_display::_update_model()
{
    b_background.position.storage = {{-1, -1}, {1, -1}, {-1, 1}, {1, 1}};
    b_background.tex_coord.storage = {{0, 0}, {1, 0}, {0, 1}, {1, 1}};
    b_background.elements.storage = {0, 1, 2, 2, 3, 0};
    b_background.update();

    set_dirty();
}

void grid_display::_update_background(void* image)
{
    gl::use_guard _g(tex_prog);
    gl::bind_vao_guard _a(b_background.va);
    glActiveTexture(GL_TEXTURE0);
    gl::bind_texture_guard _t(GL_TEXTURE_2D, b_background.tex);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); // or GL_CLAMP_TO_BORDER
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, world_size.x, world_size.y, 0, GL_RGBA,
                 GL_UNSIGNED_BYTE, image);

    set_dirty();
}

void grid_display::_update_viewport()
{
    using glm::vec2;
    using glm::vec3;

    vec2 vp_rel_size = vec2(viewport_size) / vec2(world_size);
    vec2 vp_rel_pos = vec2(_viewport_position) / vec2(world_size);
    glm::mat4 tex_view_matrix =
        glm::translate(vec3(vp_rel_pos, 0)) *
        glm::scale(vec3(invert<1>(vp_rel_size), 0)) *
        glm::translate(vec3(-.5, -.5, 0));

    gl::set_uniform(tex_prog, "viewMatrix", tex_view_matrix);
    gl::set_uniform(quad_prog, "viewportPosition", _viewport_position);
    set_uniform(quad_prog, "viewportSize", viewport_size);

    set_dirty();
}
