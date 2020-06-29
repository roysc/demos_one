# find_package(fmt 6.0 REQUIRED)

set(_HEADERS
    geometry.hpp
    rendering.hpp
    skeleton.hpp
)
set(_SOURCES
    geometry.cpp)

add_library(atrium ${_HEADERS} ${_SOURCES})
target_link_libraries(atrium
    Rxt::Graphics Rxt::Geometry
    glm::glm
)
