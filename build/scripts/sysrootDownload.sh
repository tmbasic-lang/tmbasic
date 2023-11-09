#!/bin/sh
# inputs: $ARCH
# run from the build/ directory
set -eu

export SYSROOT_VERSION="unknown_arch"
export BUILDX_ARCH="unknown_arch"

if [ "$ARCH" = "arm32v7" ]; then
    export SYSROOT_VERSION=$(awk -F= '{ if ($1 == "linux-system-root-arm32") print $2 }' tags.ini)
    export BUILDX_ARCH="linux/arm/v7"
fi
if [ "$ARCH" = "arm64v8" ]; then
    export SYSROOT_VERSION=$(awk -F= '{ if ($1 == "linux-system-root-arm64") print $2 }' tags.ini)
    export BUILDX_ARCH="linux/arm/v8"
fi
if [ "$ARCH" = "x86_64" ]; then
    export SYSROOT_VERSION=$(awk -F= '{ if ($1 == "linux-system-root-x64") print $2 }' tags.ini)
    export BUILDX_ARCH="linux/amd64"
fi
if [ "$ARCH" = "i686" ]; then
    export SYSROOT_VERSION=$(awk -F= '{ if ($1 == "linux-system-root-x86") print $2 }' tags.ini)
    export BUILDX_ARCH="linux/386"
fi

export IMAGE_NAME="ghcr.io/electroly/tmbasic-linux-sysroot:$SYSROOT_VERSION"
export FILENAME="downloads/sysroot-$SYSROOT_VERSION.tar.gz"

mkdir -p downloads
if [ ! -f "downloads/sysroot-$SYSROOT_VERSION.tar.gz" ]; then
    echo "Pulling $IMAGE_NAME..."
    docker pull --platform $BUILDX_ARCH $IMAGE_NAME

    echo "Saving to tar.gz..."
    docker save $IMAGE_NAME | gzip > "$FILENAME"
fi
cp -f "$FILENAME" "files/sysroot-$ARCH.tar.gz"
