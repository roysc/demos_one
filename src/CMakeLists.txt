# Plaza source root CMakeLists.txt

set(OpenGL_GL_PREFERENCE GLVND)
find_package(Rxt 0.2 REQUIRED
    COMPONENTS Graphics Geometry
    NO_CMAKE_FIND_ROOT_PATH)
find_package(EnTT REQUIRED)

include(RxtEmscriptenHelpers)
# include(CMakePrintHelpers)
# cmake_print_variables(CMAKE_PREFIX_PATH)

if(EMSCRIPTEN)
    add_compile_options(-fno-diagnostics-color)
    add_link_options(-fno-diagnostics-color)
endif()

set(_HEADERS
    map.hpp
    tiles.hpp
    util.hpp
    mouse_tools.hpp
    mouse_core.hpp
    viewport.hpp
    controls.hpp
    noise.hpp
    entity.hpp
    space.hpp
    geometry.hpp
    mesh.hpp
    rendering.hpp
    skeleton.hpp
)
set(_SOURCES
    entity.cpp
    geometry.cpp
    mesh.cpp
)

add_library(atrium ${_HEADERS} ${_SOURCES})
target_include_directories(atrium PUBLIC ${PROJECT_SOURCE_DIR}/external)
target_link_libraries(atrium
    Rxt::Graphics Rxt::Geometry
    glm::glm
    EnTT::EnTT
)

add_executable(map_editor ${_HEADERS} ${_SOURCES}
    old/interface_display.hpp
    old/texture_display.cpp
    old/texture_display.hpp
    old/map_editor.hpp
    old/map_editor.cpp
)
target_link_libraries(map_editor PUBLIC atrium)
Rxt_emscripten_use_boost(map_editor)

add_executable(canvas ${_HEADERS} canvas.hpp canvas.cpp)
target_link_libraries(canvas PUBLIC atrium)

add_executable(dirt ${_HEADERS}
    dirt.hpp
    dirt_signals_ui.cpp
    dirt_signals_model.cpp
    dirt_controls.cpp
    dirt.cpp
    dirt_main.cpp
)
target_link_libraries(dirt PUBLIC atrium)

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
