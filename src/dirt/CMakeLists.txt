
set(dirt_HEADERS
    dirt_models.hpp
    dirt.hpp
    rendering.hpp
    space.hpp
    entity.hpp
)
set(dirt_SOURCES
    dirt_models.cpp
    dirt.cpp
    dirt.init.cpp
    space.cpp
    entity.cpp
)

add_executable(dirt ${dirt_HEADERS} ${dirt_SOURCES})
target_link_libraries(dirt PUBLIC app3d)
target_include_directories(dirt PUBLIC ..)
