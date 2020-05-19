#pragma once

#include "viewport.hpp"
#include <Rxt/graphics/color.hpp>
#include <Rxt/range.hpp>
#include <Rxt/util.hpp>

#include <optional>

template <class P>
struct mouse_tool
{
    virtual void mouse_down(int) = 0;
    virtual void mouse_up(int) = 0;
};

template <class Tr>
struct mouse_ui
{
    // using viewport_type = viewport<P, Obs>;
    using viewport_type = viewport<Tr>;
    using P = typename Tr::vec_type;

    P _cursor_position {0};
    viewport_type* _viewport;
    mouse_ui(viewport_type& v) : _viewport{&v} {}

    P cursor_position() const {return _cursor_position;}
    void cursor_position(P p) {_cursor_position = p;}
    // P& cursor_reference() const {return *_cursor_position;}

    viewport_type const& viewport() const { return *_viewport; };
};

// This is designed to function as both a basic cursor observable state
// And a mouse tool used by vpointer.
template <class Tr>
struct mouse_select_tool
    : public mouse_ui<Tr>
    , public mouse_tool<typename Tr::vec_type>
{
    using P = typename Tr::vec_type;
    using Obs = typename Tr::observable_type;
    using region = std::tuple<P, P>;

    using mouse_ui<Tr>::mouse_ui;
    using mouse_ui<Tr>::cursor_position;
    using mouse_ui<Tr>::viewport;

    std::optional<P> drag_origin;
    std::optional<region> selection;
    Rxt::rgba color = {1, 0, 1, 0.3};

    Obs _hook_motion_;
    Obs _hook_selection_;

    void mouse_motion(P pos)
    {
        cursor_position(pos);
        _hook_motion_.notify_all();
    }

    void mouse_down(int i) override
    {
        switch (i) {
        case 0:
            drag_origin = cursor_position() + viewport().position();
            break;
        }
        _hook_motion_.notify_all();
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
                _hook_motion_.notify_all();
                _hook_selection_.notify_all();
            }
            break;
        case 1:
            if (drag_origin) {
                drag_origin.reset();
                _hook_motion_.notify_all();
            } else if (selection) {
                selection = {};
                _hook_selection_.notify_all();
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

template <class Tr>
struct mouse_paint_tool : mouse_tool<Tr>
{
    using P = typename Tr::vec_type;
    using paint_method = std::function<void(P, int)>;

    mouse_ui<Tr>& ui;
    paint_method _paint;

    mouse_paint_tool(mouse_ui<Tr>& u, paint_method m = {})
        : ui{u}, _paint{m} {}

    void set_method(paint_method m) {_paint = m;}

    void mouse_down(int i) override { if (_paint) _paint(ui.cursor_position(), i); }
    void mouse_up(int i) override { }
};
