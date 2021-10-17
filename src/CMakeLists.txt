# Atrium source root CMakeLists.txt

include(CMakePrintHelpers)

set(OpenGL_GL_PREFERENCE GLVND)

if(USE_RXT_SUBDIR)
    add_subdirectory(${PROJECT_SOURCE_DIR}/external/Rxt ${PROJECT_BINARY_DIR}/Rxt)
else()
    find_package(Rxt 0.2 REQUIRED
        COMPONENTS Graphics Geometry
        NO_CMAKE_FIND_ROOT_PATH
    )
endif()
include(RxtEmscriptenHelpers)

# set(CMAKE_FIND_DEBUG_MODE TRUE)
find_package(glm REQUIRED ${RXT_EMSCRIPTEN_FIND_ROOT_PATH_MODE})
find_package(EnTT REQUIRED ${RXT_EMSCRIPTEN_FIND_ROOT_PATH_MODE})

set(_HEADERS_BASE
    map.hpp
    tiles.hpp
    util.hpp
    mouse_core.hpp
    palette.hpp
    stage.hpp
)

function(_configure_emscripten TGT)
    if(NOT EMSCRIPTEN)
        return()
    endif()

    set_target_properties(${TGT} PROPERTIES SUFFIX ".html")
    install(DIRECTORY "${PROJECT_SOURCE_DIR}/data/" DESTINATION ${PROJECT_BINARY_DIR})

    set(_emscripten_options "SHELL:-s USE_BOOST_HEADERS=1")
    target_compile_options(${TGT} PUBLIC ${_emscripten_options})
    target_link_options(${TGT} PUBLIC ${_emscripten_options})

    Rxt_emscripten_html_shell(${TGT})
endfunction()

add_subdirectory(canvas)

add_executable(garden garden.cpp)
target_link_libraries(garden PUBLIC Rxt::Graphics Rxt::Data)
if(EMSCRIPTEN)
    _configure_emscripten(garden)
endif()

if(NOT DISABLE_3D)
    set(_HEADERS_APP3D
        app3d.hpp
        geometry.hpp
        geometry_mesh.hpp)
    set(_SOURCES_APP3D
        app3d_ui.cpp
        app3d_controls.cpp
        app3d.cpp
        entity.cpp
        geometry.cpp
        geometry_mesh.cpp)

    add_library(app3d_base ${_HEADERS_BASE} ${_SOURCES_BASE})
    target_link_libraries(app3d_base Rxt::Graphics)

    add_library(app3d ${_HEADERS_APP3D} ${_SOURCES_APP3D})
    target_include_directories(app3d PUBLIC ${PROJECT_SOURCE_DIR}/external)
    target_link_libraries(app3d
        app3d_base
        Rxt::Geometry
        EnTT::EnTT)

    add_subdirectory(dirt)
    add_subdirectory(old)
endif()
