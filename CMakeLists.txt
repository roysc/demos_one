# "plaza" demo
cmake_minimum_required(VERSION 3.8)
project(GridQuest VERSION 0.1)

if(IS_DIRECTORY external/Rxt)
    add_subdirectory(external/Rxt)
else()
    find_package(Rxt)
endif()

# set(CMAKE_CXX17_STANDARD_COMPILE_OPTION "-std=c++17")
# set(CMAKE_CXX20_STANDARD_COMPILE_OPTION "-std=c++2a")

# Global options
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin)

add_subdirectory(src)
