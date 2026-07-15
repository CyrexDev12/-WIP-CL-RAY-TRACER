if(CLRT_BUILD_TESTS)
    add_executable(clrt_math_tests core/MathTypesTests.cpp)
    target_link_libraries(clrt_math_tests PRIVATE CLRT::Math)

    add_executable(clrt_migration_tests core/MathMigrationTests.cpp)
    target_link_libraries(clrt_migration_tests PRIVATE CLRT::Core)

    add_executable(clrt_scene_tests core/SceneTests.cpp)
    target_link_libraries(clrt_scene_tests PRIVATE CLRT::Core)

    add_executable(clrt_world_ownership_tests core/WorldOwnershipTests.cpp)
    target_link_libraries(clrt_world_ownership_tests PRIVATE CLRT::Loaders)

    add_executable(clrt_mesh_asset_tests core/MeshAssetTests.cpp)
    target_link_libraries(clrt_mesh_asset_tests PRIVATE CLRT::Core)

    add_executable(clrt_mesh_instance_tests core/MeshInstanceTests.cpp)
    target_link_libraries(clrt_mesh_instance_tests PRIVATE CLRT::Core)

    add_executable(clrt_scene_snapshot_tests core/SceneSnapshotTests.cpp)
    target_link_libraries(clrt_scene_snapshot_tests PRIVATE CLRT::Core)

    add_executable(clrt_extended_loader_tests core/ExtendedSceneLoaderTests.cpp)
    target_link_libraries(clrt_extended_loader_tests PRIVATE CLRT::Loaders)

    if(MSVC)
        target_compile_options(clrt_math_tests PRIVATE /W4 /permissive-)
        target_compile_options(clrt_migration_tests PRIVATE /W4 /permissive-)
        target_compile_options(clrt_scene_tests PRIVATE /W4 /permissive-)
        target_compile_options(clrt_world_ownership_tests PRIVATE /W4 /permissive-)
        target_compile_options(clrt_mesh_asset_tests PRIVATE /W4 /permissive-)
        target_compile_options(clrt_mesh_instance_tests PRIVATE /W4 /permissive-)
        target_compile_options(clrt_scene_snapshot_tests PRIVATE /W4 /permissive-)
        target_compile_options(clrt_extended_loader_tests PRIVATE /W4 /permissive-)
    else()
        target_compile_options(clrt_math_tests PRIVATE -Wall -Wextra -Wpedantic)
        target_compile_options(clrt_migration_tests PRIVATE -Wall -Wextra -Wpedantic)
        target_compile_options(clrt_scene_tests PRIVATE -Wall -Wextra -Wpedantic)
        target_compile_options(clrt_world_ownership_tests PRIVATE -Wall -Wextra -Wpedantic)
        target_compile_options(clrt_mesh_asset_tests PRIVATE -Wall -Wextra -Wpedantic)
        target_compile_options(clrt_mesh_instance_tests PRIVATE -Wall -Wextra -Wpedantic)
        target_compile_options(clrt_scene_snapshot_tests PRIVATE -Wall -Wextra -Wpedantic)
        target_compile_options(clrt_extended_loader_tests PRIVATE -Wall -Wextra -Wpedantic)
    endif()

    add_test(NAME core.fixed_math COMMAND clrt_math_tests)
    add_test(NAME core.math_migration COMMAND clrt_migration_tests)
    add_test(NAME core.scene_ownership COMMAND clrt_scene_tests)
    add_test(
        NAME core.world_ownership
        COMMAND clrt_world_ownership_tests
            "${CMAKE_CURRENT_SOURCE_DIR}/fixtures/raii_scene.json"
            "${CMAKE_CURRENT_SOURCE_DIR}/fixtures/invalid_raii_scene.json"
    )
    add_test(NAME core.mesh_asset COMMAND clrt_mesh_asset_tests)
    add_test(NAME core.mesh_instance COMMAND clrt_mesh_instance_tests)
    add_test(NAME core.scene_snapshot COMMAND clrt_scene_snapshot_tests)
    add_test(
        NAME core.extended_scene_loader
        COMMAND clrt_extended_loader_tests
            "${CMAKE_CURRENT_SOURCE_DIR}/fixtures/extended_scene.json"
    )

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
