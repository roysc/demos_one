#include "basic3d.hpp"
#include "util.hpp"

void basic3d_app::_init_controls()
{
    using Rxt::print;
    using Rxt::fvec3;
    using Ax = Rxt::axis3;
    float speed = 0.04;

    auto put_camera = [this] (fvec3 relpos)
    {
        auto focus = camera.focus;
        auto pos = focus + relpos;
        auto up = fvec3(0,0,1);
        auto fwdxup = cross(focus - pos, -up);
        if (all(glm::epsilonEqual(fwdxup, fvec3(0), 0.001f)))
            up = fvec3(0,1,0);
        camera.emplace(pos, focus, up);
    };
    auto camera_forward = [=, this] (float dist)
    {
        camera.forward(dist);
        auto distance_threshold = 0.001f;
        if (length(camera.offset()) < distance_threshold) {
            print("Resetting degenerate camera position...\n");
            reset_camera();
        }
    };

    keys.on_press["D"] = on_debug;
    keys.on_press["C-W"] = [this] { quit = true; };
    keys.on_scan["Right"] = std::bind(orbit_cam, &camera, Ax::z, +speed);
    keys.on_scan["Left"] = std::bind(orbit_cam, &camera, Ax::z, -speed);
    keys.on_scan["Up"] = std::bind(orbit_cam, &camera, Ax::y, +speed);
    keys.on_scan["Down"] = std::bind(orbit_cam, &camera, Ax::y, -speed);
    keys.on_scan[","] = std::bind(camera_forward, +speed);
    keys.on_scan["."] = std::bind(camera_forward, -speed);
    keys.on_press["R"] = std::bind(&basic3d_app::reset_camera, this);
    keys.on_press["X"] = std::bind(put_camera, 10.f*Rxt::basis3<fvec3>(Rxt::axis3::x));
    keys.on_press["Y"] = std::bind(put_camera, 10.f*Rxt::basis3<fvec3>(Rxt::axis3::y));
    keys.on_press["Z"] = std::bind(put_camera, 10.f*Rxt::basis3<fvec3>(Rxt::axis3::z));

    PZ_observe(input.on_quit) { quit = true; };
    PZ_observe(input.on_key_down, SDL_Keysym k) { keys.press(k); };
    PZ_observe(input.on_mouse_motion, SDL_MouseMotionEvent motion) {
        auto [x, y] = sdl::nds_coords(*window, motion.x, motion.y);
        cursor.position({x, y});
    };

    input.on_mouse_wheel += [=, this] (SDL_MouseWheelEvent wheel) {
        if (wheel.y != 0)
            camera_forward(wheel.y);
        // if (wheel.x != 0)
        //     camera.orbit(glm::angleAxis(wheel.x*speed, Rxt::basis3<fvec3>(Ax::z)));
    };

    PZ_observe(input.on_mouse_down, SDL_MouseButtonEvent button) {
        switch (button.button) {
        case SDL_BUTTON_MIDDLE:
            drag_origin = {cursor.position(), camera};
            break;
        }
    };
    PZ_observe(input.on_mouse_up, SDL_MouseButtonEvent button) {
        switch (button.button) {
        case SDL_BUTTON_MIDDLE:
            drag_origin = {};
            break;
        }
    };
}
