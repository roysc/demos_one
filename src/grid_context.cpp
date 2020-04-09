#include "grid_context.hpp"

grid_context::grid_context(const char* title, grid_size world_size, glm::uvec2 tile_px)
    : simple_gui(title, tile_px * world_size)
    , world_size(world_size)
    , viewport_size(world_size)
    , tile_size_px(tile_px)
{}

void grid_context::h_scale_viewport(int exp)
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

void grid_context::update_texture(void* image)
{
    gl::use_guard _g(tex_prog);
    gl::bind_vao_guard _a(b_texs.va);
    glActiveTexture(GL_TEXTURE0);
    gl::bind_texture_guard _t(GL_TEXTURE_2D, b_texs.tex);

    if constexpr (is_torus()) {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    } else {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    }
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, world_size.x, world_size.y, 0, GL_RGBA,
                 GL_UNSIGNED_BYTE, image);

    set_dirty();
}

void grid_context::update_model()
{
    b_texs.position.storage = {{-1, -1}, {1, -1}, {-1, 1}, {1, 1}};
    b_texs.tex_coord.storage = {{0, 0}, {1, 0}, {0, 1}, {1, 1}};
    b_texs.elements.storage = {0, 1, 2, 2, 3, 0};
    b_texs.update();

    gl::set_uniform(quad_prog, "worldSize", world_size);
    gl::set_uniform(quad_prog, "doRepeat", is_torus());

    set_dirty();
}

void grid_context::update_viewport()
{
    using glm::vec2;
    using glm::vec3;

    vec2 vp_rel_size = vec2(viewport_size) / vec2(world_size);
    vec2 vp_rel_pos = vec2(viewport_position) / vec2(world_size);
    glm::mat4 tex_view_matrix =
        glm::translate(vec3(vp_rel_pos, 0)) *
        glm::scale(vec3(invert_y(vp_rel_size), 0)) *
        glm::translate(vec3(-.5, -.5, 0));

    gl::set_uniform(tex_prog, "viewMatrix", tex_view_matrix);
    gl::set_uniform(quad_prog, "viewportPosition", viewport_position);
    gl::set_uniform(quad_prog, "viewportScale", vec2(viewport_size) / 2.f);

    set_dirty();
}
