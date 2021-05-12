
set(canvas_HEADERS
    canvas.hpp
    mouse_tools.hpp
)

add_executable(canvas canvas.cpp)
target_link_libraries(canvas PUBLIC app3d_base)
target_include_directories(canvas PUBLIC ..)

_configure_emscripten(canvas)
