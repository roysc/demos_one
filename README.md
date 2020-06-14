Plaza
=====

Stage for game & ui dev

Building
========
Dependencies:
- Boost (multi_array)
- {fmt}
- tcb/span

CMake:

    $ cmake ..
    $ make

Emscripten:

    $ emconfigure cmake .. -D CMAKE_PREFIX_PATH=$HOME/.local_em
    $ emcmake cmake ..
