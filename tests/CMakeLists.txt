if(CLRT_BUILD_TESTS)
    add_executable(clrt_math_tests core/MathTypesTests.cpp)
    target_link_libraries(clrt_math_tests PRIVATE CLRT::Math)

    add_executable(clrt_migration_tests core/MathMigrationTests.cpp)
    target_link_libraries(clrt_migration_tests PRIVATE CLRT::Core)

    if(MSVC)
        target_compile_options(clrt_math_tests PRIVATE /W4 /permissive-)
        target_compile_options(clrt_migration_tests PRIVATE /W4 /permissive-)
    else()
        target_compile_options(clrt_math_tests PRIVATE -Wall -Wextra -Wpedantic)
        target_compile_options(clrt_migration_tests PRIVATE -Wall -Wextra -Wpedantic)
    endif()

    add_test(NAME core.fixed_math COMMAND clrt_math_tests)
    add_test(NAME core.math_migration COMMAND clrt_migration_tests)

    add_test(
        NAME baseline.example_scene
        COMMAND raytracer --scene "${PROJECT_SOURCE_DIR}/scenes/example_scene.json"
    )
    set_tests_properties(
        baseline.example_scene
        PROPERTIES
            WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
            TIMEOUT 120
    )
endif()

if(CLRT_BUILD_LEGACY_TESTS)
    add_executable(
        clrt_legacy_tests
        legacy_main.cpp
        legacy/Tests.cpp
    )
    target_include_directories(clrt_legacy_tests PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")
    target_link_libraries(clrt_legacy_tests PRIVATE CLRT::Core CLRT::CpuRenderer)

    if(MSVC)
        target_compile_options(clrt_legacy_tests PRIVATE /W4 /permissive-)
    else()
        target_compile_options(clrt_legacy_tests PRIVATE -Wall -Wextra -Wpedantic)
    endif()
endif()
