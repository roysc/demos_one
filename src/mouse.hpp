#pragma once

template <class P>
struct mouse_tool
{
    virtual P get_position() const = 0;
    virtual void mouse_down(int) = 0;
    virtual void mouse_up(int) = 0;
    virtual void mouse_motion(P) = 0;
};

template <class P>
struct empty_mouse_tool : mouse_tool<P>
{
    virtual P get_position() const {return {0};};
    void mouse_down(int) override {};
    void mouse_up(int) override {};
    void mouse_motion(P) override {};
};

template <class P>
struct mouse_cursor_tool : mouse_tool<P>//grid_mouse_tool
{
    P position {0};
    const Rxt::rgba cursor_color {0, 1, 1, .5};

    observable<mouse_cursor_tool> hook_cursor;

    P get_position() const override { return position; }

    void mouse_motion(P pos) override
    {
        position = pos;
        hook_cursor.notify_all(*this);
    }

    void mouse_down(int) override { }
    void mouse_up(int) override { }

    template <class UIbuf>
    void update_cursor(UIbuf& b) const
    {
        b.set_cursor(position, uvec{1}, cursor_color);
    }
};

template <class P>
struct mouse_select_tool : mouse_tool<P>
{
    using region = std::tuple<P, P>;

    grid_viewport* viewport {};

    P position {0};
    std::optional<P> drag_origin;
    std::optional<region> selection;
    Rxt::rgba color = {1, 0, 1, 0.3};

    observable<mouse_select_tool> hook_cursor;
    observable<mouse_select_tool> hook_selection;

    mouse_select_tool(grid_viewport* v) : viewport{v} {}

    P get_position() const override { return position; }

    void mouse_motion(P pos) override
    {
        position = pos;
        hook_cursor.notify_all(*this);
    }

    void mouse_down(int i) override
    {
        switch (i) {
        case 0:
            drag_origin = position + viewport->position;
            break;
        }
        hook_cursor.notify_all(*this);
    }

    void mouse_up(int i) override
    {
        switch (i) {
        case 0:
            if (drag_origin) {
                auto abspos = position + viewport->position;
                auto [a, b] = Rxt::box(*drag_origin, abspos);
                selection = {a, b};
                drag_origin = {};
                hook_cursor.notify_all(*this);
                hook_selection.notify_all(*this);
            }
            break;
        case 1:
            if (drag_origin) {
                drag_origin.reset();
                hook_cursor.notify_all(*this);
            } else if (selection) {
                selection = {};
                hook_selection.notify_all(*this);
            }
            break;
        }
    }

    template <class UIbuf>
    void update_cursor(UIbuf& bc) const
    {
        if (drag_origin) {
            auto [a, b] = Rxt::box(position, *drag_origin - viewport->position);
            bc.set_cursor(a, b-a+1, color);
        } else {
            bc.set_cursor(position, uvec{1}, color);
        }
    }

    template <class Objbuf>
    void update_selection(Objbuf& bs) const
    {
        for (auto [a, b]: Rxt::to_range(selection)) {
            bs.add_selection(a, b);
        }
    }
};
