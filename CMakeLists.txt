cmake_minimum_required(VERSION 3.31)
project(tmbasic)

# Generate compilation database for clang-tidy
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Set C++ standard to C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Define build options
option(STRIP_TMBASIC "Strip the tmbasic executable" OFF)
option(DISABLE_SANITIZERS "Disable sanitizers in debug builds" OFF)

# Define platform-specific variables
set(TARGET_OS "" CACHE STRING "Target OS (linux, win, mac)")
set(LINUX_DISTRO "" CACHE STRING "Linux distribution (alpine, ubuntu)")
set(LINUX_TRIPLE "" CACHE STRING "Linux triple for cross-compilation")
set(ARCH "" CACHE STRING "Architecture (i686, x86_64, arm32v7, arm64v8)")
set(TARGET_PREFIX "" CACHE PATH "Path to dependencies")

# Check if required variables are set
if(NOT TARGET_OS)
    message(FATAL_ERROR "TARGET_OS must be specified (linux, win, mac)")
endif()
if(TARGET_OS STREQUAL "linux")
    if(NOT LINUX_DISTRO)
        message(FATAL_ERROR "LINUX_DISTRO must be specified for Linux builds (alpine, ubuntu)")
    endif()
    if(NOT LINUX_TRIPLE)
        message(FATAL_ERROR "LINUX_TRIPLE must be specified for Linux builds (e.g., x86_64-linux-gnu)")
    endif()
endif()
if(NOT ARCH)
    message(FATAL_ERROR "ARCH must be specified (i686, x86_64, arm32v7, arm64v8)")
endif()
if(NOT TARGET_PREFIX)
    message(FATAL_ERROR "TARGET_PREFIX must be specified")
endif()

# Determine SHORT_ARCH based on ARCH
if(ARCH STREQUAL "i686")
    set(SHORT_ARCH "x86")
elseif(ARCH STREQUAL "x86_64")
    set(SHORT_ARCH "x64")
elseif(ARCH STREQUAL "arm32v7")
    set(SHORT_ARCH "arm32")
elseif(ARCH STREQUAL "arm64v8")
    set(SHORT_ARCH "arm64")
endif()

# Set RUNTIME_OUTPUT_DIRECTORY to ensure binaries are placed in the bin directory
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Variable flags.
if(TARGET_OS STREQUAL "win")
    set(NCURSES_LIB "")
    set(PTHREAD_LIB "")
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        set(TVISION_LIB "tvision-debug")
    else()
        set(TVISION_LIB "tvision")
    endif()
endif()
if(TARGET_OS STREQUAL "mac")
    set(NCURSES_LIB "ncurses")
    set(PTHREAD_LIB "pthread")
    set(TVISION_LIB "tvision")
endif()
if(TARGET_OS STREQUAL "linux")
    set(NCURSES_LIB "ncursesw")
    set(PTHREAD_LIB "pthread")
    set(TVISION_LIB "tvision")
endif()

# Handle sanitizers
if(NOT DISABLE_SANITIZERS AND TARGET_OS STREQUAL "linux" AND LINUX_DISTRO STREQUAL "ubuntu")
    # Disable sanitizers on WSL ARM64
    if(LINUX_TRIPLE STREQUAL "aarch64-linux-gnu")
        execute_process(
            COMMAND uname -r
            OUTPUT_VARIABLE UNAME_OUTPUT
            OUTPUT_STRIP_TRAILING_WHITESPACE
        )
        if(UNAME_OUTPUT MATCHES "WSL")
            set(DISABLE_SANITIZERS ON)
        endif()
    endif()
    
    if(NOT DISABLE_SANITIZERS)
        add_compile_options(
            -fsanitize=undefined
            -fsanitize=address
            -fsanitize=shift
            -fsanitize=shift-exponent
            -fsanitize=shift-base
            -fsanitize=integer-divide-by-zero
            -fsanitize=unreachable
            -fsanitize=null
            -fsanitize=return
            -fsanitize=signed-integer-overflow
            -fsanitize=bounds
            -fsanitize=float-divide-by-zero
            -fsanitize=float-cast-overflow
            -fsanitize=bool
            -fsanitize=enum
            -fsanitize=vptr
            -fsanitize=pointer-overflow
            -fsanitize=builtin
            -fno-sanitize-recover
        )
        add_link_options(-static-libasan)
    endif()
endif()

# Find source files
file(GLOB_RECURSE COMPILER_SRC_FILES src/compiler/*.cpp)
file(GLOB_RECURSE RUNNER_SRC_FILES src/runner/*.cpp)
file(GLOB_RECURSE VM_SRC_FILES src/vm/*.cpp)
file(GLOB_RECURSE SHARED_SRC_FILES src/shared/*.cpp)
file(GLOB_RECURSE TEST_SRC_FILES src/test/*.cpp)
file(GLOB_RECURSE TMBASIC_SRC_FILES src/tmbasic/*.cpp)

# Define compiler and linker flags based on target OS
if(TARGET_OS STREQUAL "win")
    set(COMMON_COMPILE_OPTIONS
        /utf-8
        /Zc:__cplusplus     # Enable proper __cplusplus macro
        /Zc:preprocessor    # Enable standards-conforming preprocessor
        /EHsc               # Standard C++ exception handling
        /permissive-        # Strict standards conformance
        /W3                 # Warning level
        /wd4996             # Disable deprecation warnings
    )
    # Add preprocessor definitions needed for MSVC compatibility
    add_compile_definitions(
        _CRT_SECURE_NO_WARNINGS
        NOMINMAX            # Don't define min/max macros
        WIN32_LEAN_AND_MEAN # Trim Windows headers
        FMT_CONSTEXPR=inline
        FMT_USE_NONTYPE_TEMPLATE_PARAMETERS=0
    )
else()
    set(COMMON_COMPILE_OPTIONS
        -Wall
        -Werror
        -Winvalid-pch
        -Wno-unknown-pragmas
        -Wno-reorder
    )
    if(TARGET_OS STREQUAL "linux")
        list(APPEND COMMON_COMPILE_OPTIONS -Wno-psabi)
    endif()
endif()

# Include and library directories
include_directories(src)
include_directories(SYSTEM ${TARGET_PREFIX}/include)
set(CMAKE_FIND_ROOT_PATH ${TARGET_PREFIX})
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

# Create static libraries
add_library(compiler STATIC ${COMPILER_SRC_FILES})
target_compile_options(compiler PRIVATE ${COMMON_COMPILE_OPTIONS})
target_compile_definitions(compiler PRIVATE UTF8PROC_STATIC)
target_precompile_headers(compiler PRIVATE src/common.h)

add_library(shared STATIC ${SHARED_SRC_FILES})
target_compile_options(shared PRIVATE ${COMMON_COMPILE_OPTIONS})
target_compile_definitions(shared PRIVATE UTF8PROC_STATIC)
target_precompile_headers(shared PRIVATE src/common.h)

add_library(vm STATIC ${VM_SRC_FILES})
target_compile_options(vm PRIVATE ${COMMON_COMPILE_OPTIONS})
target_compile_definitions(vm PRIVATE UTF8PROC_STATIC)
target_precompile_headers(vm PRIVATE src/common.h)

# Test executable
add_executable(test ${TEST_SRC_FILES})
target_compile_options(test PRIVATE ${COMMON_COMPILE_OPTIONS})
target_compile_definitions(test PRIVATE UTF8PROC_STATIC)
target_precompile_headers(test PRIVATE src/common.h)
target_link_directories(test PRIVATE ${TARGET_PREFIX}/lib)
target_link_libraries(test
    compiler shared vm
    absl_time absl_time_zone absl_int128 absl_raw_logging_internal
    microtar mpdec mpdec++ utf8proc
    turbo-core fmt ${TVISION_LIB} ${NCURSES_LIB}
    zip z gtest gtest_main ${PTHREAD_LIB}
)
if(TARGET_OS STREQUAL "linux" AND LINUX_DISTRO STREQUAL "alpine")
    target_link_options(test PRIVATE ${CMAKE_EXE_LINKER_FLAGS} -static)
endif()

# Runtime executable
add_executable(runtime ${RUNNER_SRC_FILES})
# The runner is a single .cpp file, it's faster NOT to use a precompiled header.
target_compile_options(runtime PRIVATE ${COMMON_COMPILE_OPTIONS})
target_compile_definitions(runtime PRIVATE UTF8PROC_STATIC)
target_link_directories(runtime PRIVATE ${TARGET_PREFIX}/lib)
target_link_libraries(runtime
    vm shared
    absl_time absl_time_zone absl_int128 absl_raw_logging_internal
    microtar mpdec mpdec++ utf8proc
    turbo-core fmt ${TVISION_LIB} ${NCURSES_LIB}
)

if(TARGET_OS STREQUAL "linux" AND LINUX_DISTRO STREQUAL "alpine")
    target_link_options(runtime PRIVATE ${CMAKE_EXE_LINKER_FLAGS} -static)
endif()

if(STRIP_TMBASIC)
    add_custom_command(
        TARGET runtime POST_BUILD
        COMMAND ${CMAKE_STRIP} $<TARGET_FILE:runtime>
        COMMAND chmod -x $<TARGET_FILE:runtime>
    )
endif()

# Windows resource compilation
if(TARGET_OS STREQUAL "win")
    set(WIN_RC_FILE ${CMAKE_SOURCE_DIR}/src/tmbasic/AppWin.rc)
    set(TMBASIC_RES_OBJ ${WIN_RC_FILE})
else()
    set(TMBASIC_RES_OBJ "")
endif()

# TMBASIC executable
add_executable(tmbasic ${TMBASIC_SRC_FILES} ${TMBASIC_RES_OBJ})
target_compile_options(tmbasic PRIVATE ${COMMON_COMPILE_OPTIONS})
target_compile_definitions(tmbasic PRIVATE UTF8PROC_STATIC)
target_precompile_headers(tmbasic PRIVATE src/common.h)
add_dependencies(tmbasic runtime)
target_link_directories(tmbasic PRIVATE ${TARGET_PREFIX}/lib)
target_link_libraries(tmbasic
    compiler shared
    absl_time absl_time_zone absl_int128 absl_raw_logging_internal
    microtar mpdec mpdec++ utf8proc
    turbo-core fmt ${TVISION_LIB} ${NCURSES_LIB}
    zip z
)
if(TARGET_OS STREQUAL "linux")
    if(LINUX_DISTRO STREQUAL "ubuntu")
        target_link_libraries(tmbasic ${PTHREAD_LIB})
        target_link_directories(tmbasic PRIVATE /usr/local/${LINUX_TRIPLE}/lib)
    endif()
    if(LINUX_DISTRO STREQUAL "alpine")
        target_link_options(tmbasic PRIVATE ${CMAKE_EXE_LINKER_FLAGS} -static)
        target_link_directories(tmbasic PRIVATE /target-sysroot/usr/local/lib)
    endif()
elseif(TARGET_OS STREQUAL "mac")
    target_link_directories(tmbasic PRIVATE ${TARGET_PREFIX}/lib)
    add_custom_command(
        TARGET tmbasic POST_BUILD
        COMMAND fileicon set $<TARGET_FILE:tmbasic> ${CMAKE_SOURCE_DIR}/doc/art/favicon/favicon.icns
    )
elseif(TARGET_OS STREQUAL "win")
    target_link_libraries(tmbasic shlwapi)
    target_link_libraries(test shlwapi)
    target_link_libraries(runtime shlwapi)
endif()

if(STRIP_TMBASIC)
    add_custom_command(
        TARGET tmbasic POST_BUILD
        COMMAND ${CMAKE_STRIP} $<TARGET_FILE:tmbasic>
    )
endif()

# Copy tzdb.tar to tzdb.dat in the executable directory
add_custom_command(
    TARGET tmbasic POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy ${TARGET_PREFIX}/share/tzdb.tar $<TARGET_FILE_DIR:tmbasic>/tzdb.dat
)

# Rename the runtime. Don't try to simply set this as the target's output name because on Windows it'll end up with an
# extra .exe extension.
add_custom_command(
    TARGET runtime POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E rename $<TARGET_FILE:runtime> ${CMAKE_BINARY_DIR}/bin/runtime_${TARGET_OS}_${SHORT_ARCH}.dat
)
