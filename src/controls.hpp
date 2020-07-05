#pragma once

#include "viewport.hpp"
#include "reactive.hpp"
#include "events.hpp"

#include <Rxt/graphics/camera.hpp>

// ST = spatial_traits

template <class ST>
struct basic_cursor
{
    using P = typename ST::position_type;
    P _position {0};
    P position() const { return _position; }
};

using Rxt::reactive_base;

template <class Der, class ST>
struct reactive_cursor : basic_cursor<ST>, reactive_base<Der>
{
    using super_type = basic_cursor<ST>;
    using super_type::super_type;
    using super_type::position;
    using P = typename ST::position_type;

    void position(P p) { this->_position = p; this->do_update(); }
};

template <class Der, class ST>
struct reactive_viewport : basic_viewport<ST>, reactive_base<Der>
{
    using super_type = basic_viewport<ST>;
    using super_type::super_type;
    using super_type::position;

    using P = typename ST::position_type;
    using Size = typename ST::size_type;

    void position(P p) override { super_type::position(p); this->do_update(); }
    void set_scale(Size s) override { super_type::set_scale(s); this->do_update(); }
};

template <class Der, class Cam = Rxt::focus_cam>
struct reactive_cam : Cam, reactive_base<Der>
{
    using super_type = Cam;
    using super_type::super_type;
    using super_type::position;
    reactive_cam(super_type c) : super_type(c) {}

    void position(super_type::position_type pos) override
    {
        super_type::position(pos);
        this->do_update();
    }
};

// controls position context
template <class ST>
struct cursor_port
{
    using position_type = typename ST::position_type;
    using P = position_type;
    // virtual P cursor(ST::space) const = 0;
    // virtual P viewport(ST::space) const = 0;

    virtual ~cursor_port() {}
    virtual P cursor_viewspace() const = 0;
    P cursor_worldspace() const { return cursor_viewspace() + viewport_worldspace(); }
    virtual P viewport_worldspace() const = 0;

    auto from_world(P w) const { return w - viewport_worldspace(); }
};

template <class ST>
struct controls_2d : cursor_port<ST>
{
    using super_type = cursor_port<ST>;
    using P = typename ST::position_type;
    using Size = typename ST::size_type;

    using cursor_type = basic_cursor<ST>;
    using viewport_type = basic_viewport<ST>;

    cursor_type& _cursor;
    viewport_type& _viewport;

    controls_2d(cursor_type& c, viewport_type& v) : _cursor(c), _viewport(v) {}
    P cursor_viewspace() const override { return _cursor.position(); }
    P viewport_worldspace() const override { return _viewport.position(); }
};

// template <class ST>
// struct camera_controls : controls<ST>
// {
//     using cursor_type = basic_cursor<ST>;
//     using camera_type = Rxt::focus_cam;

//     cursor_type& _cursor;
//     camera_type& _camera;

//     P cursor_position() const override { return _cursor.position(); }
//     P viewport_worldspace() const override { return _camera.position(); }
// };
