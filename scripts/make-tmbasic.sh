#!/bin/bash
# Required variables: TARGET_OS, ARCH, PREFIX
# Required when TARGET_OS=linux: LINUX_DISTRO, LINUX_TRIPLE
set -euxo pipefail

# Change to the repository root.
cd "$( dirname "${BASH_SOURCE[0]}" )"
cd ..

# Avoid bash error about uninitialized variables when building for non-Linux.
if [[ "$TARGET_OS" != "linux" ]]; then
    LINUX_DISTRO=""
    LINUX_TRIPLE=""
fi

mkdir -p cmake
cd cmake

# If TARGET_OS is "linux" and LINUX_DISTRO is "ubuntu", then we need to use the "-dev" toolchain suffix.
TOOLCHAIN_SUFFIX=""
if [[ "$TARGET_OS" == "linux" && "$LINUX_DISTRO" == "ubuntu" ]]; then
    TOOLCHAIN_SUFFIX="-dev"
fi

cmake \
    "-DCMAKE_TOOLCHAIN_FILE=../build/files/cmake-toolchain-${TARGET_OS}-${ARCH}${TOOLCHAIN_SUFFIX}.cmake" \
    -DCMAKE_VERBOSE_MAKEFILE=OFF \
    -DTARGET_OS=$TARGET_OS \
    -DLINUX_DISTRO=$LINUX_DISTRO \
    -DLINUX_TRIPLE=$LINUX_TRIPLE \
    -DARCH=$ARCH \
    "-DTARGET_PREFIX=$PREFIX" \
    "$@" \
    ..

cmake --build .
