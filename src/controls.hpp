#pragma once

#include "viewport.hpp"
#include "observable.hpp"
#include "events.hpp"

template <class ST>
struct basic_cursor
{
    using P = typename ST::position_type;
    P _position {0};
    P position() const { return _position; }
};

template <class Der>
struct reactive_base
{
    void _update() { static_cast<Der&>(*this).on_update(); };
};

template <class Der, class ST>
struct reactive_cursor : basic_cursor<ST>, reactive_base<Der>
{
    using P = typename ST::position_type;
    using basic_cursor<ST>::position;
    using basic_cursor<ST>::basic_cursor;

    void position(P p) { this->_position = p; this->_update(); }
};

template <class Der, class ST>
struct reactive_viewport : basic_viewport<ST>, reactive_base<Der>
{
    using P = typename ST::position_type;
    using Size = typename ST::size_type;
    using basic_viewport<ST>::basic_viewport;

    void position(P p) { this->_position = p; this->_update(); }
    void set_scale(Size s) { this->scale_factor = s; this->_update(); }
};

// controls position context
template <class ST>
struct control_port
{
    using P = typename ST::position_type;
    using cursor_type = basic_cursor<ST>;
    using viewport_type = basic_viewport<ST>;

    cursor_type& _cursor;
    viewport_type& _viewport;

    P cursor_position() const { return _cursor.position(); }

    P cursor_position_world() const
    {
        return cursor_position() + _viewport.position();
    }

    auto from_world(P w) const { return w - _viewport.position(); }

    // viewport_type& viewport() { return _viewport; }
    // viewport_type const& viewport() const { return _viewport; }
};
