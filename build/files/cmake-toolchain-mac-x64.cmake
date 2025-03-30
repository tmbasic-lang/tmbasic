set(CMAKE_SYSTEM_NAME Darwin)
set(CMAKE_SYSTEM_PROCESSOR x86_64)

# specify the compiler
set(CMAKE_C_COMPILER clang)
set(CMAKE_CXX_COMPILER clang++)

# Set macOS specific flags
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -arch x86_64 -mmacosx-version-min=10.13")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -arch x86_64 -mmacosx-version-min=10.13")

# Set include directories
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -isystem ${PREFIX}/include")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -isystem ${PREFIX}/include")

# Set OSX deployment target
set(CMAKE_OSX_DEPLOYMENT_TARGET "10.13" CACHE STRING "Minimum OS X deployment version")
set(CMAKE_OSX_ARCHITECTURES "x86_64" CACHE STRING "Build architectures for macOS")

# skip framework checks
set(CMAKE_FIND_FRAMEWORK NEVER) 