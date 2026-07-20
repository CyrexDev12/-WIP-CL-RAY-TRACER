find_package(Threads REQUIRED)

find_package(nlohmann_json CONFIG QUIET)

if(NOT TARGET nlohmann_json::nlohmann_json)
    set(_nlohmann_json_hint "")
    if(DEFINED ENV{CLRT_NLOHMANN_JSON_INCLUDE_DIR} AND NOT "$ENV{CLRT_NLOHMANN_JSON_INCLUDE_DIR}" STREQUAL "")
        set(_nlohmann_json_hint "$ENV{CLRT_NLOHMANN_JSON_INCLUDE_DIR}")
    elseif(CLRT_NLOHMANN_JSON_INCLUDE_DIR)
        set(_nlohmann_json_hint "${CLRT_NLOHMANN_JSON_INCLUDE_DIR}")
    endif()

    set(_nlohmann_json_candidates
        "${_nlohmann_json_hint}"
        "C:/msys64/mingw64/include"
        "C:/msys64/mingw32/include"
        "C:/msys64/mingw64/include/nlohmann"
        "C:/vcpkg/installed/x64-mingw-dynamic/include"
        "C:/vcpkg/installed/x64-mingw-static/include"
        "C:/vcpkg/installed/x86-mingw-dynamic/include"
        "C:/vcpkg/installed/x86-mingw-static/include"
    )

    foreach(_candidate IN LISTS _nlohmann_json_candidates)
        if(_candidate AND EXISTS "${_candidate}/nlohmann/json.hpp")
            set(CLRT_NLOHMANN_JSON_INCLUDE_DIR "${_candidate}")
            break()
        endif()
    endforeach()

    if(NOT CLRT_NLOHMANN_JSON_INCLUDE_DIR)
        find_path(
            CLRT_NLOHMANN_JSON_INCLUDE_DIR
            NAMES nlohmann/json.hpp
            HINTS ${_nlohmann_json_candidates}
            DOC "Directory containing nlohmann/json.hpp"
        )
    endif()

    if(NOT CLRT_NLOHMANN_JSON_INCLUDE_DIR)
        message(FATAL_ERROR
            "nlohmann/json.hpp was not found. Install nlohmann-json (for example with vcpkg: "
            "vcpkg install nlohmann-json:x64-mingw-dynamic) or set CLRT_NLOHMANN_JSON_INCLUDE_DIR "
            "to its include directory."
        )
    endif()

    add_library(nlohmann_json::nlohmann_json INTERFACE IMPORTED)
    set_target_properties(
        nlohmann_json::nlohmann_json
        PROPERTIES
            INTERFACE_INCLUDE_DIRECTORIES "${CLRT_NLOHMANN_JSON_INCLUDE_DIR}"
    )
endif()
