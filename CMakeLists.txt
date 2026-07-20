cmake_minimum_required(VERSION 3.24)

project(CLRayTracer VERSION 0.1.0 LANGUAGES CXX)

option(CLRT_BUILD_TESTS "Build and register baseline tests" ON)
option(CLRT_BUILD_LEGACY_TESTS "Build the legacy manual test executable" OFF)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(WIN32 AND NOT DEFINED CMAKE_CXX_STANDARD_INCLUDE_DIRECTORIES)
    set(CMAKE_CXX_STANDARD_INCLUDE_DIRECTORIES
        "C:/msys64/mingw64/include"
        "C:/msys64/ucrt64/include"
        "C:/msys64/mingw32/include"
    )
endif()

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(Dependencies)

add_subdirectory(src)

if(CLRT_BUILD_TESTS OR CLRT_BUILD_LEGACY_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
