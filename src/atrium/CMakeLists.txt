# find_package(fmt 6.0 REQUIRED)

set(_HEADERS
    geometry.hpp
    interaction.hpp
    rendering.hpp
)
add_library(atrium ${_HEADERS})
target_link_libraries(atrium Rxt::Graphics Rxt::Geometry)
