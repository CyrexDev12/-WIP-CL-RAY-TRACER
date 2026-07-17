find_package(Threads REQUIRED)
if(CLRT_BUILD_OPENGL)
    find_package(OpenGL REQUIRED)
    find_package(glfw3 CONFIG REQUIRED)

    if(WIN32 AND NOT DEFINED Python_EXECUTABLE)
        set(Python_FIND_REGISTRY LAST)
    endif()

    find_package(Python REQUIRED COMPONENTS Interpreter)
    include(FetchContent)

    FetchContent_Declare(
        glad
        GIT_REPOSITORY https://github.com/Dav1dde/glad.git
        GIT_TAG v2.0.8
        GIT_SHALLOW TRUE
        SOURCE_SUBDIR cmake
    )

    FetchContent_MakeAvailable(glad)

    glad_add_library(
        clrt_glad
        STATIC
        REPRODUCIBLE
        API gl:core=3.3
    )
endif()

find_package(nlohmann_json CONFIG QUIET)

if(NOT TARGET nlohmann_json::nlohmann_json)
    find_path(
        CLRT_NLOHMANN_JSON_INCLUDE_DIR
        NAMES nlohmann/json.hpp
        DOC "Directory containing nlohmann/json.hpp"
    )

    if(NOT CLRT_NLOHMANN_JSON_INCLUDE_DIR)
        message(FATAL_ERROR
            "nlohmann/json.hpp was not found. Install nlohmann-json or set "
            "CLRT_NLOHMANN_JSON_INCLUDE_DIR to its include directory."
        )
    endif()

    add_library(nlohmann_json::nlohmann_json INTERFACE IMPORTED)
    set_target_properties(
        nlohmann_json::nlohmann_json
        PROPERTIES
            INTERFACE_INCLUDE_DIRECTORIES "${CLRT_NLOHMANN_JSON_INCLUDE_DIR}"
    )
endif()
