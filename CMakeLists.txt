cmake_minimum_required(VERSION 3.17)
project(Atrium VERSION 0.1.1)

set(RXT_AS_SUBDIR "" CACHE STRING
    "Path to Rxt to use as a source subdirectory instead of package")

set(CMAKE_FIND_USE_PACKAGE_REGISTRY FALSE)

# Global options
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_compile_options(-Wall -Wextra)
add_compile_options(-Werror=return-type)

add_subdirectory(src)
