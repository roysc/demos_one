#pragma once

#include "viewport.hpp"
#include <Rxt/graphics/color.hpp>
#include <Rxt/range.hpp>
#include <Rxt/util.hpp>

#include <optional>

struct mouse_tool
{
    virtual void mouse_down(int) = 0;
    virtual void mouse_up(int) = 0;
};

// template <class Uitr>
template <class GT>
struct mouse_ui
{
    // using viewport_type = viewport<P, Obs>;
    // using P = typename GT::vec_type;
    using P = typename GT::position_type;
    using viewport_type = viewport<GT>;

    P _cursor_position {0};
    viewport_type* _viewport;
    mouse_ui(viewport_type& v) : _viewport{&v} {}

    P cursor_position() const {return _cursor_position;}
    void cursor_position(P p) {_cursor_position = p;}
    // P& cursor_reference() const {return *_cursor_position;}

    viewport_type const& viewport() const { return *_viewport; };
};

namespace tags {
struct cursor_motion {};
struct cursor_selection {};
}

// This is designed to function as both a basic cursor observable state
// And a mouse tool used by vpointer.
template <class GT>
struct mouse_select_tool
    : public mouse_ui<GT>
    , public mouse_tool
{
    using P = typename GT::position_type;
    using cursor_motion_tag = tags::cursor_motion;
    using cursor_selection_tag = tags::cursor_selection;

    using region = std::tuple<P, P>;

    using MouseUi = mouse_ui<GT>;
    using MouseUi::MouseUi;
    using MouseUi::cursor_position;
    using MouseUi::viewport;

    std::optional<P> drag_origin;
    std::optional<region> selection;
    Rxt::rgba color = {1, 0, 1, 0.3};

    lazy_subject<cursor_motion_tag> _hook_motion;
    lazy_subject<cursor_selection_tag> _hook_selection;

    auto& get_subject(cursor_motion_tag) {return _hook_motion;}
    auto& get_subject(cursor_selection_tag) {return _hook_selection;}

    template <class O>
    void set_router(O& obr)
    {
        obr.set_subject(cursor_motion_tag{}, _hook_motion);
        obr.set_subject(cursor_selection_tag{}, _hook_selection);
    }

    void mouse_motion(P pos)
    {
        cursor_position(pos);
        _hook_motion();
    }

    void mouse_down(int i) override
    {
        switch (i) {
        case 0:
            drag_origin = cursor_position() + viewport().position();
            break;
        }
        _hook_motion();
    }

    void mouse_up(int i) override
    {
        switch (i) {
        case 0:
            if (drag_origin) {
                auto abspos = cursor_position() + viewport().position();
                auto [a, b] = Rxt::box(*drag_origin, abspos);
                selection = {a, b};
                drag_origin = {};
                _hook_motion();
                _hook_selection();
            }
            break;
        case 1:
            if (drag_origin) {
                drag_origin.reset();
                _hook_motion();
            } else if (selection) {
                selection = {};
                _hook_selection();
            }
            break;
        }
    }

    template <class UIbuf>
    void update_cursor(UIbuf& buf) const
    {
        if (drag_origin) {
            auto [a, b] = Rxt::box(cursor_position(), *drag_origin - viewport().position());
            buf.set_cursor(a, b-a+1, color);
        } else {
            buf.set_cursor(cursor_position(), uvec{1}, color);
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
    // using P = typename GT::vec_type;
    using paint_method = std::function<void(typename GT::position_type, int)>;

    using MouseUi = mouse_ui<GT>;
    MouseUi& ui;
    paint_method _paint;

    mouse_paint_tool(MouseUi& u, paint_method m = {})
        : ui{u}, _paint{m} {}

    void set_method(paint_method m) {_paint = m;}

    void mouse_down(int i) override { if (_paint) _paint(ui.cursor_position(), i); }
    void mouse_up(int i) override { }
};
