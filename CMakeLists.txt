cmake_minimum_required(VERSION 3.17)
project(DemosOne VERSION 0.1.1)

set(RXT_AS_SUBDIR "" CACHE STRING
    "[DEV] Path to Rxt to use as a source subdirectory instead of package")

# If RXT_AS_SUBDIR is passed, link to it from external and treat it as a subdirectory
if(NOT RXT_AS_SUBDIR STREQUAL "")
    file(REAL_PATH "${RXT_AS_SUBDIR}" RXT_AS_SUBDIR)
    if(NOT IS_DIRECTORY "${RXT_AS_SUBDIR}")
        message(SEND_ERROR "RXT_AS_SUBDIR directory does not exist: '${RXT_AS_SUBDIR}'")
    endif()
endif()

set(CMAKE_FIND_USE_PACKAGE_REGISTRY FALSE)

# Global options
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_compile_options(-Wall -Wextra)
add_compile_options(-Werror=return-type)

add_subdirectory(src)
