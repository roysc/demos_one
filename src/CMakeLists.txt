# Atrium source root CMakeLists.txt

set(OpenGL_GL_PREFERENCE GLVND)

if(USE_RXT_SUBDIR)
    add_subdirectory(${PROJECT_SOURCE_DIR}/external/Rxt ${PROJECT_BINARY_DIR}/Rxt)
else()
    find_package(Rxt 0.2 REQUIRED
        COMPONENTS Graphics Geometry
        NO_CMAKE_FIND_ROOT_PATH
    )
endif()

find_package(EnTT REQUIRED)
find_package(glm REQUIRED)

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
    space.hpp
    app3d.hpp
    palette.hpp
    stage.hpp
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

set(_DIRT_HEADERS
    dirt_models.hpp
    dirt.hpp
)
set(_DIRT_SOURCES
    dirt_models.cpp
    dirt.cpp
    dirt_init.cpp
    dirt_main.cpp
)
add_executable(dirt ${_DIRT_HEADERS} ${_DIRT_SOURCES})
target_link_libraries(dirt PUBLIC app3d)

function(_configure_emscripten _TARGET)
    if(NOT EMSCRIPTEN)
        return()
    endif()

    set_target_properties(${_TARGET} PROPERTIES SUFFIX ".html")
    install(DIRECTORY "${PROJECT_SOURCE_DIR}/data/" DESTINATION ${PROJECT_BINARY_DIR})

    set(_emscripten_options "SHELL:-s USE_BOOST_HEADERS=1")
    target_compile_options(${_TARGET} PUBLIC ${_emscripten_options})
    target_link_options(${_TARGET} PUBLIC ${_emscripten_options})
    # target_link_options(${_TARGET} PUBLIC --emrun) #needed?
endfunction()

add_subdirectory(old)
add_subdirectory(canvas)

add_executable(garden garden.cpp)
target_link_libraries(garden PUBLIC Rxt::Graphics Rxt::Data)
