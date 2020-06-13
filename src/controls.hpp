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

template <class Der, class ST>
struct reactive_cursor : basic_cursor<ST>
{
    void position(P p) { _position = p; _update(); }
    void _update() { static_cast<Der&>(*this).on_update(); };
};

// stupid wrapper fixme
template <class ST>
struct control_port
{
    using P = typename V::position_type;
    using cursor_type = C;
    using viewport_type = V;

    basic_cursor<ST>& _cursor;
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
