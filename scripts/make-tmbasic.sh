#!/bin/bash
# Required variables: TARGET_OS, LINUX_DISTRO, LINUX_TRIPLE, ARCH
set -e

# Change to the repository root.
cd "$( dirname "${BASH_SOURCE[0]}" )"
cd ..

mkdir -p cmake
cd cmake

# If TARGET_OS is "linux" and LINUX_DISTRO is "ubuntu", then we need to use the "-dev" suffix.
SUFFIX=""
if [[ "$TARGET_OS" == "linux" && "$LINUX_DISTRO" == "ubuntu" ]]; then
    SUFFIX="-dev"
fi

cmake \
    -DCMAKE_TOOLCHAIN_FILE=../build/files/cmake-toolchain-${TARGET_OS}-${ARCH}${SUFFIX}.cmake \
    -DCMAKE_VERBOSE_MAKEFILE=OFF \
    -DTARGET_OS=$TARGET_OS \
    -DLINUX_DISTRO=$LINUX_DISTRO \
    -DLINUX_TRIPLE=$LINUX_TRIPLE \
    -DARCH=$ARCH \
    "$@" \
    ..

cmake --build .
