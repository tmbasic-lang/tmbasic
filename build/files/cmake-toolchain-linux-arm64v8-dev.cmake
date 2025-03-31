set(CMAKE_SYSTEM_NAME Linux)
set(TRIPLE "aarch64-linux-gnu")
set(CMAKE_C_COMPILER gcc)
set(CMAKE_CXX_COMPILER g++)
include_directories(SYSTEM /usr/local/${TRIPLE}/include /usr/include/${TRIPLE} ${PREFIX}/include)
set(CMAKE_LINKER ld)
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17")
