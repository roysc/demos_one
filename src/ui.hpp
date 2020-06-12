#pragma once

#include "viewport.hpp"
#include "observable.hpp"
#include "events.hpp"

template <class GT>
struct observable_viewport : public viewport<GT>
{
    using Super = viewport<GT>;
    using P = typename GT::position_type;
    observable<tags::viewport_tag> on_change;

    using Super::Super;
    observable_viewport(Super const& v) : Super{v} {}
    // auto& get_subject(tags::viewport_tag) { return on_change; }

    void scale(int exp) override { Super::scale(exp); on_change(); }
    void move(P d) override { Super::move(d); on_change(); }
};

template <class P>
struct observable_cursor
{
    P _position {0};
    observable<tags::cursor_motion_tag> on_change;

    P position() const { return _position; }
    void position(P p) { _position = p; on_change(); }
};

// stupid wrapper
template <class GT>
struct control_port
{
    using P = typename GT::position_type;
    using cursor_type = observable_cursor<P>;
    using viewport_type = observable_viewport<GT>;

    cursor_type& _cursor;
    viewport_type& _viewport;

    cursor_type& cursor() { return _cursor; }
    cursor_type const& cursor() const { return _cursor; }
    viewport_type& viewport() { return _viewport; }
    viewport_type const& viewport() const { return _viewport; }

    P cursor_position_world() const
    {
        return _cursor.position() + _viewport.position();
    }
};
