
set(canvas_HEADERS
    canvas.hpp
    mouse_tools.hpp
)

add_executable(canvas canvas.cpp)
target_link_libraries(canvas PUBLIC app3d_base)

_configure_emscripten(canvas)
