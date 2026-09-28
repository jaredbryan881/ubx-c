cmake_minimum_required(VERSION 3.16)
project(ubx-c VERSION 0.1.0 LANGUAGES C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(UBX_BUILD_TESTS "Build unit tests" ON)
option(UBX_BUILD_FUZZ "Build libFuzzer targets (clang only)" OFF)
option(UBX_WARNINGS_AS_ERRORS "Treat compiler warnings as errors" OFF)
option(UBX_SANITIZE "Build with AddressSanitizer and UndefinedBehaviorSanitizer" OFF)
option(UBX_COVERAGE "Build with coverage instrumentation" OFF)

set(UBX_WARNING_FLAGS
    -Wall -Wextra -Wpedantic
    -Wconversion -Wsign-conversion
    -Wshadow -Wundef -Wcast-qual -Wcast-align
    -Wstrict-prototypes -Wmissing-prototypes -Wold-style-definition
    -Wredundant-decls -Wnested-externs -Wpointer-arith -Wwrite-strings
    -Wformat=2 -Wnull-dereference -Wdouble-promotion
    -Wswitch -Wswitch-default
    -Wunused-macros
)
if(UBX_WARNINGS_AS_ERRORS)
    list(APPEND UBX_WARNING_FLAGS -Werror)
endif()

add_library(ubx
    src/ubx_ack.c
    src/ubx_cfg.c
    src/ubx_encoder.c
    src/ubx_nav.c
    src/ubx_parser.c
    src/ubx_tim.c
)
target_include_directories(ubx
    PUBLIC  ${CMAKE_CURRENT_SOURCE_DIR}/include
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src
)
target_compile_options(ubx PRIVATE ${UBX_WARNING_FLAGS})

if(UBX_SANITIZE)
    set(UBX_SAN_FLAGS -fsanitize=address,undefined -fno-sanitize-recover=undefined -fno-omit-frame-pointer)
    target_compile_options(ubx PUBLIC ${UBX_SAN_FLAGS})
    target_link_options(ubx PUBLIC ${UBX_SAN_FLAGS})
endif()

if(UBX_COVERAGE)
    target_compile_options(ubx PUBLIC --coverage -O0)
    target_link_options(ubx PUBLIC --coverage)
endif()

if(UBX_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

if(UBX_BUILD_FUZZ)
    if(NOT CMAKE_C_COMPILER_ID MATCHES "Clang")
        message(FATAL_ERROR "UBX_BUILD_FUZZ requires clang")
    endif()
    add_subdirectory(fuzz)
endif()
