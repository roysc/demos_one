# Atrium source root CMakeLists.txt

include(CMakeFindDependencyMacro)

set(OpenGL_GL_PREFERENCE GLVND)
find_dependency(Rxt 0.2 REQUIRED
    COMPONENTS Graphics Geometry
    NO_CMAKE_FIND_ROOT_PATH
)
find_dependency(EnTT REQUIRED)
# if(NOT glm_DIR) #FIXME why do I need this? what's the point of find_dependency? fuck.
#     find_dependency(glm CONFIG REQUIRED)
# endif()

include(RxtEmscriptenHelpers)

if(EMSCRIPTEN)
    add_compile_options(-fno-diagnostics-color)
    add_link_options(-fno-diagnostics-color)
endif()

set(_HEADERS_BASE
    map.hpp
    tiles.hpp
    util.hpp
    mouse_tools.hpp
    mouse_core.hpp
    viewport.hpp
    controls.hpp
    space.hpp
    app3d.hpp
    palette.hpp
    stage.hpp
    reactive.hpp
)
set(_SOURCES_BASE
    app3d_ui.cpp
    app3d_controls.cpp
    app3d.cpp
    space.cpp
)

set(_HEADERS_APP3D
    geometry.hpp
    geometry_mesh.hpp
    rendering.hpp
    sprite.hpp
)
set(_SOURCES_APP3D
    entity.cpp
    geometry.cpp
    geometry_mesh.cpp
)

add_library(app3d_base ${_HEADERS_BASE} ${_SOURCES_BASE})
target_link_libraries(app3d_base
    Rxt::Graphics
    glm::glm
)

add_library(app3d ${_HEADERS_APP3D} ${_SOURCES_APP3D})
target_include_directories(app3d PUBLIC ${PROJECT_SOURCE_DIR}/external)
target_link_libraries(app3d
    app3d_base
    Rxt::Geometry
    EnTT::EnTT
)

add_executable(map_editor
    old/interface_display.hpp
    old/texture_display.cpp
    old/texture_display.hpp
    old/map_editor.hpp
    old/map_editor.cpp
)
target_link_libraries(map_editor PUBLIC app3d_base)
target_include_directories(app3d_base PUBLIC ${PROJECT_SOURCE_DIR}/external)
Rxt_emscripten_use_boost(map_editor)

add_executable(canvas canvas.hpp canvas.cpp)
target_link_libraries(canvas PUBLIC app3d_base)

set(_PLANT_HEADERS
    plant_models.hpp
    plant.hpp
)
set(_PLANT_SOURCES
    plant_models.cpp
    plant.cpp
    plant_init.cpp
    plant_main.cpp
)
add_executable(plant ${_PLANT_HEADERS} ${_PLANT_SOURCES})
target_link_libraries(plant PUBLIC app3d)

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
