# see: https://gitlab.kitware.com/cmake/community/-/wikis/doc/cmake/cross_compiling/Mingw

set(CMAKE_SYSTEM_NAME Windows)
set(COMPILER_PREFIX "i686-w64-mingw32")
set(CMAKE_SYSTEM_PROCESSOR i686)
find_program(CMAKE_RC_COMPILER NAMES ${COMPILER_PREFIX}-windres)
find_program(CMAKE_C_COMPILER NAMES ${COMPILER_PREFIX}-gcc)
find_program(CMAKE_CXX_COMPILER NAMES ${COMPILER_PREFIX}-g++)
set(CMAKE_FIND_ROOT_PATH /usr/${COMPILER_PREFIX})
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

# Set include directories for mpdec
include_directories(SYSTEM /usr/${COMPILER_PREFIX}/include/libmpdec /usr/${COMPILER_PREFIX}/include/libmpdec++)
# Disable array bounds warning
set(CMAKE_CXX_FLAGS_INIT "-Wno-array-bounds")
set(CMAKE_C_FLAGS_INIT "-Wno-array-bounds")
find_program(CMAKE_STRIP NAMES ${COMPILER_PREFIX}-strip)
