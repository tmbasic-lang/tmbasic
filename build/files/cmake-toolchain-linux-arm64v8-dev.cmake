set(CMAKE_SYSTEM_NAME Linux)
set(TRIPLE "aarch64-linux-gnu")
set(CMAKE_C_COMPILER gcc)
set(CMAKE_CXX_COMPILER g++)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -isystem /usr/local/${TRIPLE}/include -isystem /usr/include/${TRIPLE} -isystem ${PREFIX}/include")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -isystem /usr/local/${TRIPLE}/include -isystem /usr/include/${TRIPLE} -isystem ${PREFIX}/include")
set(CMAKE_LINKER ld)
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)
