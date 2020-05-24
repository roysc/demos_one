#pragma once

#include "viewport.hpp"
#include "observable.hpp"
#include "events.hpp"

#include <Rxt/graphics/color.hpp>
#include <Rxt/range.hpp>
#include <Rxt/util.hpp>

#include <optional>

struct mouse_tool
{
    virtual void mouse_down(int) = 0;
    virtual void mouse_up(int) = 0;
};

template <class GT>
struct control_port
{
    using P = typename GT::position_type;
    using ViewportBase = viewport<GT>;

    struct viewport_type : ViewportBase
    {
        using Super = ViewportBase;
        // using Super::Super;
        observable<tags::viewport> on_change;

        viewport_type(Super const& v) : Super{v} {}

        auto& get_subject(tags::viewport) { return on_change; }
        void scale(int exp) override { Super::scale(exp); on_change(); }
        void move(P d) override { Super::move(d); on_change(); }
    };
    // using viewport_type = viewport<GT>;

    viewport_type _viewport;
    P _cursor_position {0};
    lazy_observable<tags::cursor_motion> on_motion;
    observable<tags::viewport>& on_viewport_change = _viewport.on_change;

    control_port(ViewportBase v) : _viewport{v} {}

    P cursor_position() const { return _cursor_position; }
    void cursor_position(P p) { _cursor_position = p; on_motion(); }

    ViewportBase const& viewport() const { return _viewport; }
    ViewportBase& viewport() { return _viewport; }

    P world_cursor_position() const
    {
        return cursor_position() + viewport().position();
    }
};

// This is designed to function as both a basic cursor observable state
// And a mouse tool used by vpointer.
template <class GT>
struct mouse_select_tool
    : public mouse_tool
{
    using P = typename GT::position_type;
    using cursor_motion_tag = tags::cursor_motion;
    using cursor_selection_tag = tags::cursor_selection;

    using region = std::tuple<P, P>;

    // using MouseUi = mouse_ui<GT>;
    // using MouseUi::MouseUi;
    // using MouseUi::cursor_position;
    // using MouseUi::viewport;
    control_port<GT>& controls;

    std::optional<P> drag_origin;
    std::optional<region> selection;
    Rxt::rgba color = {1, 0, 1, 0.3};

    lazy_observable<cursor_motion_tag> on_motion;
    lazy_observable<cursor_selection_tag> on_selection;

    mouse_select_tool(control_port<GT>& c) : controls{c} {}

    void mouse_down(int i) override
    {
        switch (i) {
        case 0:
            drag_origin = controls.world_cursor_position();
            break;
        }
        on_motion();
    }

    void mouse_up(int i) override
    {
        switch (i) {
        case 0:
            if (drag_origin) {
                auto abspos = controls.world_cursor_position();
                auto [a, b] = Rxt::box(*drag_origin, abspos);
                selection = {a, b};
                drag_origin = {};
                on_motion();
                on_selection();
            }
            break;
        case 1:
            if (drag_origin) {
                drag_origin.reset();
                on_motion();
            } else if (selection) {
                selection = {};
                on_selection();
            }
            break;
        }
    }

    template <class UIbuf>
    void update_cursor(UIbuf& buf) const
    {
        if (drag_origin) {
            auto [a, b] = Rxt::box(controls.cursor_position(), *drag_origin - controls.viewport().position());
            buf.set_cursor(a, b-a+1, color);
        } else {
            buf.set_cursor(controls.cursor_position(), uvec{1}, color);
        }
    }

    template <class Objbuf>
    void update_selection(Objbuf& buf) const
    {
        for (auto [a, b]: Rxt::to_range(selection)) {
            buf.add_selection(a, b);
        }
    }
};

template <class GT>
struct mouse_paint_tool : mouse_tool
{
    using P = typename GT::position_type;
    using paint_method = std::function<void(P, int)>;

    control_port<GT>& controls;
    paint_method _paint;

    lazy_observable<tags::object_edit> on_edit;

    mouse_paint_tool(control_port<GT>& u, paint_method m = {})
        : controls{u}, _paint{m} {}

    void set_method(paint_method m) {_paint = m;}

    void mouse_down(int i) override { if (_paint) _paint(controls.world_cursor_position(), i); on_edit(); }
    void mouse_up(int i) override { }
};

template <class GT>
struct mouse_stroke_tool : mouse_tool
{
    using P = typename GT::position_type;
    using line = std::pair<P, P>;
    using stroke = std::vector<line>;

    control_port<GT>& controls;
    std::vector<stroke> _strokes;
    const Rxt::rgba cursor_color {Rxt::colors::yellow, 1};
    const Rxt::rgba stroke_color {Rxt::colors::white, 1};

    lazy_observable<tags::object_edit> on_edit;

    void mouse_down(int i) override {}
    void mouse_up(int i) override {}

    template <class Buf>
    void update_cursor(Buf& buf) const
    {
        P a, b;
        b = controls.cursor_position();
        if (!_strokes.empty() && !_strokes.at(0).empty()) {
            a = _strokes[0].back().second;
            buf.add_line(a, b, cursor_color);
        }
    }

    template <class Buf>
    void update_model(Buf& buf) const
    {
        for (auto& s:_strokes) {
            for (auto& [a, b]: s) {
                buf.add_line(a, b, stroke_color);
            }
        }
    }
};
