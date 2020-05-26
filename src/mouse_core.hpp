#pragma once

enum mouse_button : int
{
    left = 0, right, middle, invalid
};

struct mouse_tool
{
    virtual void mouse_down(mouse_button) = 0;
    virtual void mouse_up(mouse_button) = 0;
};
