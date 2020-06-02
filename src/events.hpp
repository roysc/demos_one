#pragma once

namespace _tag
{
struct _dbg {};
struct _on {};
struct _off {};

struct view {};
struct move {};
struct select {};
struct click {};
struct edit {};
}

namespace tags
{
using debug_tag = _tag::_dbg;
using activate_tag = _tag::_on;
using deactivate_tag = _tag::_off;

using viewport_tag = _tag::view;
using cursor_motion_tag = _tag::move;
using cursor_selection_tag = _tag::select;
using mouse_click_tag = _tag::click;
using object_edit_tag = _tag::edit;

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

