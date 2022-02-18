#include "app3d.hpp"
#include "util.hpp"

void basic_app3d::_init_controls()
{
    using Rxt::print;
    using Rxt::vec::fvec3;
    using Ax = Rxt::axis3;
    float speed = 0.04;

    auto put_camera = [this](fvec3 relpos) {
        auto focus = camera.focus;
        auto pos = focus + relpos;
        auto up = fvec3(0, 0, 1);
        auto fwdxup = cross(focus - pos, -up);
        if (all(glm::epsilonEqual(fwdxup, fvec3(0), 0.001f)))
            up = fvec3(0, 1, 0);
        camera.emplace(pos, focus, up);
    };
    auto camera_forward = [=, this](float dist) {
        camera.forward(dist);
        auto distance_threshold = 0.001f;
        if (length(camera.offset()) < distance_threshold) {
            print("Resetting degenerate camera position...\n");
            reset_camera();
        }
    };

    _keys.on_press["D"] = on_debug;
    _keys.on_press["C-W"] = [this] { quit = true; };
    _keys.on_scan["Right"] = std::bind(orbit_cam, &camera, Ax::z, +speed);
    _keys.on_scan["Left"] = std::bind(orbit_cam, &camera, Ax::z, -speed);
    _keys.on_scan["Up"] = std::bind(orbit_cam, &camera, Ax::y, +speed);
    _keys.on_scan["Down"] = std::bind(orbit_cam, &camera, Ax::y, -speed);
    _keys.on_scan[","] = std::bind(camera_forward, +speed);
    _keys.on_scan["."] = std::bind(camera_forward, -speed);
    _keys.on_press["R"] = std::bind(&basic_app3d::reset_camera, this);
    _keys.on_press["X"] = std::bind(put_camera, 10.f * Rxt::basis3<fvec3>(Rxt::axis3::x));
    _keys.on_press["Y"] = std::bind(put_camera, 10.f * Rxt::basis3<fvec3>(Rxt::axis3::y));
    _keys.on_press["Z"] = std::bind(put_camera, 10.f * Rxt::basis3<fvec3>(Rxt::axis3::z));

    RXT_observe (input.on_quit) {
        quit = true;
    };
    RXT_observe (input.on_key_down, SDL_Keysym k) {
        _keys.press(k);
    };
    RXT_observe (input.on_mouse_motion, SDL_MouseMotionEvent motion) {
        auto [x, y] = sdl::nds_coords(window(), motion.x, motion.y);
        cursor.set_position({x, y});
    };

    input.on_mouse_wheel += [=, this](SDL_MouseWheelEvent wheel) {
        if (wheel.y != 0)
            camera_forward(wheel.y);
        // if (wheel.x != 0)
        //     camera.orbit(glm::angleAxis(wheel.x*speed, Rxt::basis3<fvec3>(Ax::z)));
    };

    RXT_observe (input.on_mouse_down, SDL_MouseButtonEvent button) {
        switch (button.button) {
        case SDL_BUTTON_MIDDLE:
            drag_origin = {cursor.position(), camera};
            break;
        }
    };
    RXT_observe (input.on_mouse_up, SDL_MouseButtonEvent button) {
        switch (button.button) {
        case SDL_BUTTON_MIDDLE:
            drag_origin = {};
            break;
        }
    };
}
