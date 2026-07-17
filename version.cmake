execute_process(
    COMMAND git describe --tags --always --long --dirty
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
    OUTPUT_VARIABLE GIT_VERSION_STRING
    OUTPUT_STRIP_TRAILING_WHITESPACE
    ERROR_QUIET
)

if(NOT GIT_VERSION_STRING)
    set(GIT_VERSION_STRING "v0.0.0-unknown")
endif()

if(GIT_VERSION_STRING MATCHES "v?([0-9]+)\\.([0-9]+)\\.([0-9]+)-([0-9]+)-g([0-9a-f]+)")
    set(VERSION_MAJOR ${CMAKE_MATCH_1})
    set(VERSION_MINOR ${CMAKE_MATCH_2})
    set(VERSION_PATCH ${CMAKE_MATCH_3})
    set(VERSION_COMMIT_COUNT ${CMAKE_MATCH_4})
    set(VERSION_HASH ${CMAKE_MATCH_5})
else()
    set(VERSION_MAJOR 0)
    set(VERSION_MINOR 0)
    set(VERSION_PATCH 0)
    set(VERSION_COMMIT_COUNT 0)
    set(VERSION_HASH "debug")
endif()

configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/include/Version.hpp.in"
    "${CMAKE_CURRENT_BINARY_DIR}/generated/Version.hpp"
    @ONLY
)