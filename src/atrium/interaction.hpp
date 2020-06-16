#pragma once

#include "../reactive.hpp"
#include "geometry.hpp"

#include <optional>
#include <utility>

namespace atrium
{
enum class ux_mode { object, face };

namespace _ux
{
struct ux_data
{
    using highlight_data = std::optional<object_face_key>;
    adapt_reactive<highlight_data> highlight;
};

template <class Query>
auto face_query(mesh_data const& geom, Query query)
{
    ux_data::highlight_data ret;
    if (auto opt = geom.triangle_tree.first_intersected_primitive(query)) {
        auto [index, fd] = *opt;
        ret.emplace(index, geom.face_comaps.at(index).at(fd));
    }
    return ret;
}
}

using _ux::ux_data;
using _ux::face_query;
}
