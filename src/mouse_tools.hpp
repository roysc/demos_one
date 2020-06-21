#pragma once

#include "mouse_core.hpp"
#include "controls.hpp"
#include "events.hpp"
#include "util.hpp"
#include "reactive.hpp"

#include <Rxt/graphics/color.hpp>
#include <Rxt/range.hpp>
#include <Rxt/util.hpp>

#include <optional>

// Select a box
template <class ST>
struct mouse_select_tool
    : public mouse_tool
{
    using P = typename ST::position_type;

    using region = std::tuple<P, P>;

    cursor_port<ST>& controls;
    std::optional<P> drag_origin;
    std::optional<region> selection;
    Rxt::rgba color = {1, 0, 1, 0.3};

    Rxt::hooks<> on_motion, on_selection;

    mouse_select_tool(cursor_port<ST>& c) : controls{c} {}

    void mouse_down(mouse_button i) override
    {
        switch (i) {
        case mouse_button::left:
            drag_origin = controls.cursor_worldspace();
            break;
        default: {}
        }
        on_motion();
    }

    void mouse_up(mouse_button i) override
    {
        switch (i) {
        case mouse_button::left:
            if (drag_origin) {
                auto abspos = controls.cursor_worldspace();
                auto [a, b] = Rxt::box(*drag_origin, abspos);
                selection = {a, b};
                drag_origin = {};
                on_motion();
                on_selection();
            }
            break;
        case mouse_button::right:
            if (drag_origin) {
                drag_origin.reset();
                on_motion();
            } else if (selection) {
                selection = {};
                on_selection();
            }
            break;
        default: {}
        }
    }

    template <class UIbuf>
    void update_cursor(UIbuf& buf) const
    {
        if (drag_origin) {
            auto [a, b] = Rxt::box(controls.cursor_viewspace(), controls.from_world(*drag_origin));
            buf.set_cursor(a, b-a+1, color);
        } else {
            buf.set_cursor(controls.cursor_viewspace(), uvec{1}, color);
        }
    }

    template <class Objbuf>
    void update_selection(Objbuf& buf) const
    {
        buf.clear();
        for (auto [a, b]: Rxt::to_range(selection)) {
            buf.add_selection(a, b);
        }
        buf.update();
    }
};

template <class ST>
struct mouse_paint_tool : mouse_tool
{
    using P = typename ST::position_type;
    using paint_method = std::function<void(P, int)>;

    cursor_port<ST>& controls;
    paint_method _paint;

    Rxt::hooks<> on_edit;

    mouse_paint_tool(cursor_port<ST>& u, paint_method m = {})
        : controls{u}, _paint{m} {}

    void set_method(paint_method m) {_paint = m;}

    void mouse_down(mouse_button i) override
    {
        if (_paint)
            _paint(controls.cursor_worldspace(), i);
        on_edit();
    }

    void mouse_up(mouse_button i) override { }
};

template <class ST>
struct mouse_stroke_tool : mouse_tool
{
    using P = typename ST::position_type;
    using line = std::pair<P, P>;
    using stroke = std::vector<P>;

    cursor_port<ST>& controls;
    std::vector<stroke> _strokes;
    std::optional<stroke> _current;

    const Rxt::rgba cursor_color {Rxt::colors::yellow, 1};
    const Rxt::rgba stroke_color {Rxt::colors::white, 1};

    Rxt::hooks<> on_edit;

    mouse_stroke_tool(cursor_port<ST>& c) : controls{c} {}

    void mouse_down(mouse_button i) override
    {
        if (i == mouse_button::right) {
            finish();
            return;
        }
        if (!_current)
            _current.emplace();
        _current->push_back(controls.cursor_worldspace());
        on_edit();
    }

    void mouse_up(mouse_button i) override {}

    void finish()
    {
        if (!_current) return;
        if (_current->empty())
            return Rxt::print("ignoring empty stroke\n");

        _strokes.emplace_back(*_current);
        for (auto& p: _strokes.back()) Rxt_DEBUG(p);

        _current.reset();
        on_edit();
    }

    template <class Buf>
    void update_cursor(Buf& buf) const
    {
        buf.clear();
        if (!_current) return;

        P a = _current->back(), b = controls.cursor_worldspace();
        buf.add_line(a, b, cursor_color);
        buf.update();
    }

    template <class Buf>
    void update_model(Buf& buf) const
    {
        auto add_lines = [&](auto& s, auto color) {
            for (auto it = s.begin(); it+1 != s.end(); ++it) {
                buf.add_line(*it, *(it+1), color);
            }
        };

        buf.clear();
        if (_current)
            add_lines(*_current, cursor_color);
        for (auto& s:_strokes) {
            assert(!s.empty());
            add_lines(s, stroke_color);
        }
        buf.update();
    }
};
