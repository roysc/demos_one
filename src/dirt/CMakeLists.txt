
set(dirt_HEADERS
    dirt_models.hpp
    dirt.hpp
    rendering.hpp
    space.hpp
)
set(dirt_SOURCES
    dirt_models.cpp
    dirt.cpp
    dirt_init.cpp
    dirt_main.cpp
    space.cpp
)

add_executable(dirt ${dirt_HEADERS} ${dirt_SOURCES})
target_link_libraries(dirt PUBLIC app3d)
target_include_directories(dirt PUBLIC ..)
