#!/bin/bash
# inputs: $ARCH
# run from the build/ directory
set -euxo pipefail

export IMAGE_TAG="unknown_arch"
export BUILDX_ARCH="unknown_arch"

if [ "$ARCH" = "arm32v7" ]; then
    export IMAGE_TAG=$(awk -F= '{ if ($1 == "linux-system-root-arm32") print $2 }' tags.ini)
    export BUILDX_ARCH="linux/arm32"
fi
if [ "$ARCH" = "arm64v8" ]; then
    export IMAGE_TAG=$(awk -F= '{ if ($1 == "linux-system-root-arm64") print $2 }' tags.ini)
    export BUILDX_ARCH="linux/arm64"
fi
if [ "$ARCH" = "x86_64" ]; then
    export IMAGE_TAG=$(awk -F= '{ if ($1 == "linux-system-root-x64") print $2 }' tags.ini)
    export BUILDX_ARCH="linux/amd64"
fi
if [ "$ARCH" = "i686" ]; then
    export IMAGE_TAG=$(awk -F= '{ if ($1 == "linux-system-root-x86") print $2 }' tags.ini)
    export BUILDX_ARCH="linux/386"
fi

export IMAGE_NAME="ghcr.io/tmbasic-lang/tmbasic-linux-sysroot"
export FILENAME="downloads/sysroot-$IMAGE_TAG.tar.gz"

mkdir -p downloads
if [ ! -f "downloads/sysroot-$IMAGE_TAG.tar.gz" ]; then
    echo "Pulling $IMAGE_NAME:$IMAGE_TAG..."
    docker pull --platform "$BUILDX_ARCH" "$IMAGE_NAME:$IMAGE_TAG"

    echo "Saving to tar.gz..."
    export CONTAINER_NAME=$(docker create --platform "$BUILDX_ARCH" "$IMAGE_NAME:$IMAGE_TAG" /bin/bash)

    if [ -z "$CONTAINER_NAME" ]; then
        echo "Failed to create the container. Exiting."
        exit 1
    fi

    docker export "$CONTAINER_NAME" | gzip > "$FILENAME"

    docker rm "$CONTAINER_NAME"
fi
cp -f "$FILENAME" "files/sysroot-$ARCH.tar.gz"
