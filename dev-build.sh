#!/bin/sh
# Required variables: TARGET_OS, LINUX_DISTRO, LINUX_TRIPLE, ARCH
set -e
mkdir -p cmake
cd cmake

# Check if required variables are set
if [ -z "$TARGET_OS" ] || [ -z "$LINUX_DISTRO" ] || [ -z "$LINUX_TRIPLE" ] || [ -z "$ARCH" ]; then
    echo "Error: Required variables are not set."
    exit 1
fi

cmake \
    -DCMAKE_TOOLCHAIN_FILE=/code/build/files/cmake-toolchain-linux-arm64v8-dev.cmake \
    -DCMAKE_VERBOSE_MAKEFILE=OFF \
    -DTARGET_OS=$TARGET_OS \
    -DLINUX_DISTRO=$LINUX_DISTRO \
    -DLINUX_TRIPLE=$LINUX_TRIPLE \
    -DARCH=$ARCH \
    ..

cmake --build .
