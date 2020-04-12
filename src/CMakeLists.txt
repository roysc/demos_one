find_package(fmt 6.0 REQUIRED)

set(_HEADERS
    map_editor.hpp
    grid_display.hpp
    space.hpp
    agent.hpp
    tiles.hpp
    grid.hpp
)

set(_SOURCES
    map_editor_main.cpp
    map_editor.cpp
    grid_display.cpp
    space.cpp
)

add_executable(map_editor ${_HEADERS} ${_SOURCES})
target_link_libraries(map_editor RxtGraphics fmt::fmt)
