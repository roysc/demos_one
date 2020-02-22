find_package(fmt 6.0 REQUIRED)

set(_SOURCES
    main.cpp
    game_context.hpp
    grid_context.hpp
    space.hpp
    agent.hpp
)
add_executable(demo ${_SOURCES})
target_link_libraries(demo RxtGraphics fmt::fmt)
