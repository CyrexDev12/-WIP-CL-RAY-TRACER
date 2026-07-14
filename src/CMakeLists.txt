add_library(clrt_math STATIC core/math/Mat4.cpp)
add_library(CLRT::Math ALIAS clrt_math)
target_include_directories(clrt_math PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")

set(CLRT_CORE_SOURCES
    geometry/Computations.cpp
    geometry/Cube.cpp
    geometry/Cylinder.cpp
    geometry/Group.cpp
    geometry/Hexagon.cpp
    geometry/Intersection.cpp
    geometry/Plane.cpp
    geometry/Ray.cpp
    geometry/Shape.cpp
    geometry/Sphere.cpp
    geometry/Triangle.cpp
    math/BasicPhysics.cpp
    math/LegacyMathAdapters.cpp
    math/Matrix.cpp
    math/Operations.cpp
    scene/Camera.cpp
    scene/canvas.cpp
    scene/Lighting.cpp
    scene/LightShadeVector.cpp
    scene/Pattern.cpp
    scene/PointLight.cpp
    scene/World.cpp
)

add_library(clrt_core STATIC ${CLRT_CORE_SOURCES})
add_library(CLRT::Core ALIAS clrt_core)
target_include_directories(clrt_core PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
target_link_libraries(clrt_core PUBLIC CLRT::Math)

add_library(clrt_cpu STATIC renderers/cpu/CpuRenderer.cpp)
add_library(CLRT::CpuRenderer ALIAS clrt_cpu)
target_link_libraries(clrt_cpu PUBLIC CLRT::Core Threads::Threads)

add_library(clrt_loaders STATIC loaders/SceneLoader.cpp)
add_library(CLRT::Loaders ALIAS clrt_loaders)
target_link_libraries(clrt_loaders PUBLIC CLRT::Core nlohmann_json::nlohmann_json)

add_executable(raytracer app/main.cpp)
target_link_libraries(raytracer PRIVATE CLRT::Core CLRT::CpuRenderer CLRT::Loaders)

foreach(target clrt_math clrt_core clrt_cpu clrt_loaders raytracer)
    if(MSVC)
        target_compile_options(${target} PRIVATE /W4 /permissive-)
    else()
        target_compile_options(${target} PRIVATE -Wall -Wextra -Wpedantic)
    endif()
endforeach()
