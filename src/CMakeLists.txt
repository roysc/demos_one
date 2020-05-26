# Plaza source root CMakeLists.txt

include(RxtEmscriptenHelpers)
include(CMakePrintHelpers)
cmake_print_variables(CMAKE_PREFIX_PATH)

if(EMSCRIPTEN)
    add_compile_options(-fno-diagnostics-color)
    add_link_options(-fno-diagnostics-color)
endif()

add_library(plaza_gui
    viewport.hpp
    gui/texture_display.hpp
    gui/interface_display.hpp
    gui/texture_display.cpp
)
target_link_libraries(plaza_gui PUBLIC Rxt::Graphics)

set(_HEADERS
    map.hpp
    tiles.hpp
    util.hpp
    observable.hpp
    mouse.hpp
    mouse_core.hpp
    tool.hpp
    viewport.hpp
    control_port.hpp
)

set(_SOURCES
    map.cpp
)

add_executable(map_editor ${_HEADERS} ${_SOURCES}
    gui/map_editor.hpp
    gui/map_editor.cpp
)
target_link_libraries(map_editor PUBLIC plaza_gui)
Rxt_emscripten_use_boost(map_editor)

target_include_directories(map_editor PRIVATE ${PROJECT_SOURCE_DIR}/src)
target_include_directories(map_editor PRIVATE ${PROJECT_SOURCE_DIR}/external)

add_executable(canvas ${_HEADERS}
    canvas.hpp canvas.cpp
)
target_link_libraries(canvas PUBLIC plaza_gui)

function(_configure_emscripten _TARGET)
    set_target_properties(${_TARGET} PROPERTIES SUFFIX ".html")
    install(DIRECTORY "${PROJECT_SOURCE_DIR}/data/" DESTINATION ${PROJECT_BINARY_DIR})

    set(_emscripten_options "SHELL:-s USE_BOOST_HEADERS=1")
    target_compile_options(${_TARGET} PUBLIC ${_emscripten_options})
    target_link_options(${_TARGET} PUBLIC ${_emscripten_options})
    # target_link_options(${_TARGET} PUBLIC --emrun) #needed?
endfunction()

if(EMSCRIPTEN)
    _configure_emscripten(map_editor)
    _configure_emscripten(canvas)
endif()
