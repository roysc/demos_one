#pragma once

namespace tags
{
struct debug_tag {};
struct activate_tag {};
struct deactivate_tag {};

struct viewport_tag {};
struct cursor_motion_tag {};
struct cursor_selection_tag {};
struct mouse_click_tag {};
struct object_edit_tag {};

// convenience
constexpr debug_tag debug;
constexpr activate_tag activate;
constexpr deactivate_tag deactivate;

constexpr viewport_tag viewport;
constexpr cursor_motion_tag cursor_motion;
constexpr cursor_selection_tag cursor_selection;
constexpr mouse_click_tag mouse_click;
constexpr object_edit_tag object_edit;
}

