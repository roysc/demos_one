find_package(fmt 6.0 REQUIRED)

set(_HEADERS
    game_context.hpp
    grid_context.hpp
    space.hpp
    agent.hpp
)

set(_SOURCES
    main.cpp
    game_context.cpp
    grid_context.cpp
    space.cpp
)

add_executable(demo ${_HEADERS} ${_SOURCES})
target_link_libraries(demo RxtGraphics fmt::fmt)
