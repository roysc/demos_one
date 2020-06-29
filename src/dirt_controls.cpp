#include "dirt.hpp"
#include "util.hpp"

void dirt_app::_init_controls()
{
    using Rxt::print;
    using Ax = Rxt::axis3;
    float speed = 0.04;

    auto reset_camera = [this] { camera.emplace(start_camera_at); };

    keys.on_scan["Right"] = std::bind(orbit_cam, &camera, Ax::z, +speed);
    keys.on_scan["Left"] = std::bind(orbit_cam, &camera, Ax::z, -speed);
    keys.on_scan["Up"] = std::bind(orbit_cam, &camera, Ax::y, +speed);
    keys.on_scan["Down"] = std::bind(orbit_cam, &camera, Ax::y, -speed);
    keys.on_scan[","] = [=, this] { camera.forward(+speed); };
    keys.on_scan["."] = [=, this] { camera.forward(-speed); };
    keys.on_press["C-W"] = [this] { quit = true; };
    keys.on_press["D"] = on_debug;
    keys.on_press["R"] = reset_camera;

    PZ_observe(input.on_quit) { quit = true; };
    PZ_observe(input.on_key_down, SDL_Keysym k) { keys.press(k); };
    PZ_observe(input.on_mouse_motion, SDL_MouseMotionEvent motion) {
        auto [x, y] = sdl::nds_coords(*window, motion.x, motion.y);
        cursor.position({x, y});
    };

    PZ_observe(input.on_mouse_wheel, SDL_MouseWheelEvent wheel) {
        if (wheel.y != 0)
            camera.forward(wheel.y);
        // if (wheel.x != 0)
        //     camera.orbit(glm::angleAxis(wheel.x*speed, Rxt::basis3<fvec3>(Ax::z)));
    };

    PZ_observe(input.on_mouse_down, SDL_MouseButtonEvent button) {
        switch (button.button) {
        case SDL_BUTTON_LEFT: {
            if (auto pos = selected_space()) {
                put_body(entreg, *pos, cpt::build_plant());
                ent_update();
            }
            break;
        }
        case SDL_BUTTON_MIDDLE:
            drag_origin = cursor.position();
            // if (drag_origin) print("drag_origin = {}\n", *drag_origin); 
            break;
        case SDL_BUTTON_RIGHT:
            if (auto pos = selected_space()) {
                put_body(entreg, *pos, cpt::build_man());
                ent_update();
            }
            break;
        }
    };
    PZ_observe(input.on_mouse_up, SDL_MouseButtonEvent button) {
        switch (button.button) {
        case SDL_BUTTON_MIDDLE:
            // drag_origin = controls.cursor_position_world();
            drag_origin = {};
            break;
        }        
    };
}
