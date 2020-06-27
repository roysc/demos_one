#pragma once

#include "../reactive.hpp"
#include "geometry.hpp"

#include <optional>
#include <utility>

namespace a3um
{
enum class ux_mode { object, face };

using highlight_data = std::optional<object_face_key>;
using ux_data = highlight_data;
}
