#!/bin/bash
# Required variables: TARGET_OS, LINUX_DISTRO, LINUX_TRIPLE, ARCH, PREFIX
set -euxo pipefail

# Change to the repository root.
cd "$( dirname "${BASH_SOURCE[0]}" )"
cd ..

mkdir -p cmake
cd cmake

# If TARGET_OS is "linux" and LINUX_DISTRO is "ubuntu", then we need to use the "-dev" toolcahin suffix.
SUFFIX=""
if [[ "$TARGET_OS" == "linux" && "$LINUX_DISTRO" == "ubuntu" ]]; then
    SUFFIX="-dev"
fi

TOOLCHAIN_FILE="../build/files/cmake-toolchain-${TARGET_OS}-${ARCH}${SUFFIX}.cmake"

cmake \
    "-DCMAKE_TOOLCHAIN_FILE=$TOOLCHAIN_FILE" \
    -DCMAKE_VERBOSE_MAKEFILE=ON \
    -DTARGET_OS=$TARGET_OS \
    -DLINUX_DISTRO=$LINUX_DISTRO \
    -DLINUX_TRIPLE=$LINUX_TRIPLE \
    -DARCH=$ARCH \
    "-DPREFIX=$PREFIX" \
    "$@" \
    ..

cmake --build .
