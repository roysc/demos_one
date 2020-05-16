#pragma once

template <class P>
struct mouse_tool
{
    virtual void mouse_down(int) = 0;
    virtual void mouse_up(int) = 0;
    virtual void mouse_motion(P) = 0;
};

template <class P>
struct empty_mouse_tool : mouse_tool<P>
{
    void mouse_down(int) override {};
    void mouse_up(int) override {};
    void mouse_motion(P) override {};
};
