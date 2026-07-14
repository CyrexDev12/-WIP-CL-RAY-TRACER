find_package(Threads REQUIRED)

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
