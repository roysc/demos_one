# Plaza source root CMakeLists.txt

include(RxtEmscriptenHelpers)
include(CMakePrintHelpers)
cmake_print_variables(CMAKE_PREFIX_PATH)

if(EMSCRIPTEN)
    add_compile_options(-fno-diagnostics-color)
    add_link_options(-fno-diagnostics-color)
endif()

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
target_link_libraries(map_editor PUBLIC Rxt::Graphics)
Rxt_emscripten_link_boost(map_editor)

if(EMSCRIPTEN)
    set_target_properties(map_editor PROPERTIES SUFFIX ".html")
    install(DIRECTORY "${PROJECT_SOURCE_DIR}/data/" DESTINATION ${PROJECT_BINARY_DIR})

    set(_emscripten_options "SHELL:-s USE_BOOST_HEADERS=1")
    target_compile_options(map_editor PUBLIC ${_emscripten_options})
    target_link_options(map_editor PUBLIC ${_emscripten_options})
    # target_link_options(map_editor PUBLIC --emrun) #needed?
endif()
