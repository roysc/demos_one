#pragma once

template <class P>
struct mouse_tool
{
    virtual void mouse_down(int) = 0;
    virtual void mouse_up(int) = 0;
    virtual void mouse_motion(P) = 0;
    virtual P position() const = 0;
};

// template <class P>
// struct empty_mouse_tool : mouse_tool<P>
// {
//     void mouse_down(int) override {};
//     void mouse_up(int) override {};
//     void mouse_motion(P) override {};
//     // virtual P position() const {return {0};};
// };

// template <class P>
// struct mouse_cursor_tool //:  mouse_tool<P>//grid_mouse_tool
// {
//     P position {0};
//     const Rxt::rgba cursor_color {0, 1, 1, .5};

//     observable<mouse_cursor_tool> _hook_motion_;

//     P get_position() const override { return position; }

//     void mouse_motion(P pos) override
//     {
//         position = pos;
//         _hook_motion_.notify_all(*this);
//     }

//     // void mouse_down(int) override { }
//     // void mouse_up(int) override { }

//     template <class UIbuf>
//     void update_cursor(UIbuf& b) const
//     {
//         b.set_cursor(position, uvec{1}, cursor_color);
//     }
// };

template <class P>
struct mouse_select_tool : mouse_tool<P>
{
    using region = std::tuple<P, P>;

    grid_viewport* viewport {};

    P _position {0};
    std::optional<P> drag_origin;
    std::optional<region> selection;
    Rxt::rgba color = {1, 0, 1, 0.3};

    observable<mouse_select_tool> _hook_motion_;
    observable<mouse_select_tool> _hook_selection_;

    mouse_select_tool(grid_viewport* v) : viewport{v} {}

    P position() const override { return _position; }

    void mouse_motion(P pos) override
    {
        _position = pos;
        _hook_motion_.notify_all(*this);
    }

    void mouse_down(int i) override
    {
        switch (i) {
        case 0:
            drag_origin = _position + viewport->position();
            break;
        }
        _hook_motion_.notify_all(*this);
    }

    void mouse_up(int i) override
    {
        switch (i) {
        case 0:
            if (drag_origin) {
                auto abspos = _position + viewport->position();
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
            auto [a, b] = Rxt::box(_position, *drag_origin - viewport->position());
            buf.set_cursor(a, b-a+1, color);
        } else {
            buf.set_cursor(_position, uvec{1}, color);
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
