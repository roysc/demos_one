
add_executable(map_editor
    interface_display.hpp
    texture_display.cpp
    texture_display.hpp
    map_editor.hpp
    map_editor.cpp
)
target_link_libraries(map_editor PUBLIC app3d_base)
target_include_directories(app3d_base PUBLIC ${PROJECT_SOURCE_DIR}/external)
Rxt_emscripten_use_boost(map_editor)

_configure_emscripten(map_editor)
