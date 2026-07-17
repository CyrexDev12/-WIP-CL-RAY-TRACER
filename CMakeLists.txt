cmake_minimum_required(VERSION 3.24)

project(CLRayTracer VERSION 0.1.0 LANGUAGES CXX)

option(CLRT_BUILD_TESTS "Build and register baseline tests" ON)
option(CLRT_BUILD_LEGACY_TESTS "Build the legacy manual test executable" OFF)
option(CLRT_BUILD_OPENGL "Build OpenGL viewer support" OFF)

# Enable C language if OpenGL support is requested
if(CLRT_BUILD_OPENGL)
    enable_language(C)
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(Dependencies)

add_subdirectory(src)

if(CLRT_BUILD_TESTS OR CLRT_BUILD_LEGACY_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
