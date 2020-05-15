# Plaza source root CMakeLists.txt

include(RxtEmscriptenHelpers)
include(CMakePrintHelpers)
cmake_print_variables(CMAKE_PREFIX_PATH)

if(EMSCRIPTEN)
    add_compile_options(-fno-diagnostics-color)
    add_link_options(-fno-diagnostics-color)
endif()

add_library(plaza_gui
    gui/grid_viewport.hpp
    gui/texture_display.hpp
    gui/interface_display.hpp

    gui/grid_viewport.cpp
    gui/texture_display.cpp
)
target_link_libraries(plaza_gui PUBLIC Rxt::Graphics)

set(_HEADERS
    space.hpp
    tiles.hpp
    # modvec.hpp
    observable.hpp
)

set(_SOURCES
    space.cpp
)

add_executable(map_editor ${_HEADERS} ${_SOURCES}
    gui/map_editor.hpp
    gui/map_editor.cpp
)
target_link_libraries(map_editor PUBLIC plaza_gui)
Rxt_emscripten_link_boost(map_editor)

add_executable(canvas ${_HEADERS}
    canvas.hpp canvas.cpp
)
target_link_libraries(canvas PUBLIC plaza_gui)

target_include_directories(map_editor PRIVATE ${PROJECT_SOURCE_DIR}/src)
target_include_directories(map_editor PRIVATE ${PROJECT_SOURCE_DIR}/external)

if(EMSCRIPTEN)
    set_target_properties(map_editor PROPERTIES SUFFIX ".html")
    install(DIRECTORY "${PROJECT_SOURCE_DIR}/data/" DESTINATION ${PROJECT_BINARY_DIR})

    set(_emscripten_options "SHELL:-s USE_BOOST_HEADERS=1")
    target_compile_options(map_editor PUBLIC ${_emscripten_options})
    target_link_options(map_editor PUBLIC ${_emscripten_options})
    # target_link_options(map_editor PUBLIC --emrun) #needed?
endif()
