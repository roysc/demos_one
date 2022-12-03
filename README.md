# Atrium

Stage for game & ui dev

## Building

Dependencies:
- Boost (multi_array)
- {fmt}
- tcb/span

CMake:

    $ cmake -S. -B build
    $ make

Emscripten:

    $ emconfigure cmake .. -D CMAKE_PREFIX_PATH=$HOME/.local_em
    $ emcmake cmake ..

## Dependencies

# EnTT

`git@github.com:skypjack/entt.git`
