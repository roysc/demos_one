#pragma once

#include "viewport.hpp"
#include "observable.hpp"
#include "events.hpp"

// Basically a viewport and a cursor
// With resp. observables
template <class GT>
struct control_port
{
    using P = typename GT::position_type;
    using ViewportBase = viewport<GT>;

    struct viewport_type : ViewportBase
    {
        using Super = ViewportBase;
        observable<tags::viewport_tag> on_change;

        viewport_type(Super const& v) : Super{v} {}
        auto& get_subject(tags::viewport_tag) { return on_change; }
        void scale(int exp) override { Super::scale(exp); on_change(); }
        void move(P d) override { Super::move(d); on_change(); }
    };

    viewport_type _viewport;
    P _cursor_position {0};
    lazy_observable<tags::cursor_motion_tag> on_motion;
    observable<tags::viewport_tag>& on_viewport_change = _viewport.on_change;

    control_port(ViewportBase v) : _viewport{v} {}

    P cursor_position() const { return _cursor_position; }
    void cursor_position(P p) { _cursor_position = p; on_motion(); }

    ViewportBase const& viewport() const { return _viewport; }
    ViewportBase& viewport() { return _viewport; }

    P cursor_position_world() const
    {
        return cursor_position() + viewport().position();
    }
};
