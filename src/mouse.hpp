#pragma once

#include "viewport.hpp"

template <class P>
struct mouse_tool
{
    virtual void mouse_down(int) = 0;
    virtual void mouse_up(int) = 0;
};

template <class P>
struct mouse_ui
{
    P _cursor_position {0};
    viewport<P>* _viewport;
    mouse_ui(viewport<P>& v) : _viewport{&v} {}

    P cursor_position() const {return _cursor_position;}
    void cursor_position(P p) {_cursor_position = p;}
    // P& cursor_reference() const {return *_cursor_position;}

    viewport<P> const& viewport() const { return *_viewport; };
};

// This is designed to function as both a basic cursor observable state
// And a mouse tool used by vpointer.
template <class P>
struct mouse_select_tool
    : public mouse_ui<P>
    , public mouse_tool<P>
{
    using region = std::tuple<P, P>;

    using mouse_ui<P>::mouse_ui;
    using mouse_ui<P>::cursor_position;
    using mouse_ui<P>::viewport;

    std::optional<P> drag_origin;
    std::optional<region> selection;
    Rxt::rgba color = {1, 0, 1, 0.3};

    observable<mouse_select_tool> _hook_motion_;
    observable<mouse_select_tool> _hook_selection_;

    void mouse_motion(P pos)
    {
        cursor_position(pos);
        _hook_motion_.notify_all(*this);
    }

    void mouse_down(int i) override
    {
        switch (i) {
        case 0:
            drag_origin = cursor_position() + viewport().position();
            break;
        }
        _hook_motion_.notify_all(*this);
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
                _hook_motion_.notify_all(*this);
                _hook_selection_.notify_all(*this);
            }
            break;
        case 1:
            if (drag_origin) {
                drag_origin.reset();
                _hook_motion_.notify_all(*this);
            } else if (selection) {
                selection = {};
                _hook_selection_.notify_all(*this);
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

template <class P>
struct mouse_paint_tool : mouse_tool<P>
{
    using paint_method = std::function<void(P, int)>;

    mouse_ui<P>& ui;
    paint_method _paint;

    mouse_paint_tool(mouse_ui<P>& u, paint_method m = {})
        : ui{u}, _paint{m} {}

    void set_method(paint_method m) {_paint = m;}

    void mouse_down(int i) override { if (_paint) _paint(ui.cursor_position(), i); }
    void mouse_up(int i) override { }
};
