#pragma once

#include "../reactive.hpp"
#include "geometry.hpp"

#include <optional>
#include <utility>

namespace atrium
{
enum class ux_mode { object, face };

using highlight_data = std::optional<object_face_key>;
using ux_data = highlight_data;
}
