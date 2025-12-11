# Atrium source root CMakeLists.txt

set(OpenGL_GL_PREFERENCE GLVND)

include(CMakePrintHelpers)

if(RXT_DIR STREQUAL "")
    find_package(Rxt REQUIRED
        COMPONENTS Graphics Geometry Data
        NO_CMAKE_FIND_ROOT_PATH
    )
else()
    cmake_print_variables(RXT_DIR)
    add_subdirectory("${RXT_DIR}" ${CMAKE_BINARY_DIR}/Rxt)
endif()

if(ENTT_DIR STREQUAL "")
    find_package(EnTT REQUIRED ${RXT_EMSCRIPTEN_FIND_ROOT_PATH_MODE})
else()
    cmake_print_variables(ENTT_DIR)
    add_subdirectory("${ENTT_DIR}" ${CMAKE_BINARY_DIR}/entt)
endif()

include(RxtEmscriptenHelpers)

set(_HEADERS_BASE
    map.hpp
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

if(NOT DISABLE_3D)
    add_library(app3d)
    target_sources(app3d PRIVATE
        ${_HEADERS_BASE}
        app3d.hpp
        geometry.hpp
        geometry_mesh.hpp
    )
    target_sources(app3d PRIVATE
        ${_SOURCES_BASE}
        app3d_ui.cpp
        app3d_controls.cpp
        app3d.cpp
        geometry.cpp
        geometry_mesh.cpp
    )

    target_include_directories(app3d PUBLIC ${PROJECT_SOURCE_DIR}/external)
    target_link_libraries(app3d
        Rxt::Graphics
        Rxt::Geometry
        EnTT::EnTT
    )

    add_subdirectory(dirt)
    # add_subdirectory(old)
endif()
