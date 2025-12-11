# Demos #1

Basic 2d & 3d graphics demos

## Building

CMake:

    $ cmake -S. -B build
    $ make

Emscripten:

    $ emconfigure cmake .. -D CMAKE_PREFIX_PATH=$HOME/.local_em
    $ emcmake cmake ..

## Dependencies

- EnTT
