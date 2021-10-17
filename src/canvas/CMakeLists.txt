
set(canvas_HEADERS
    canvas.hpp
    mouse_tools.hpp
)

add_executable(canvas canvas.cpp)
target_link_libraries(canvas PUBLIC Rxt::Base)

_configure_emscripten(canvas)
