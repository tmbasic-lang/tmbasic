#!/bin/bash
# arguments: $BUILDX_ARCH $IMAGE_NAME $ARCH
set -euxo pipefail

# Change to build/sysroots
cd "$( dirname "${BASH_SOURCE[0]}" )"

# Build the container
cat Dockerfile.sysroot \
    | envsubst \
    | docker buildx build --progress plain --load --platform "$BUILDX_ARCH" -t "$IMAGE_NAME" . -f- \
    ;

# Export the root filesystem to a tarball.
SYSROOT_CONTAINER_NAME=$(docker create --platform "$BUILDX_ARCH" "$IMAGE_NAME" /bin/bash)
docker export "$SYSROOT_CONTAINER_NAME" | gzip > "../files/sysroot-$ARCH.tar.gz"
docker rm "$SYSROOT_CONTAINER_NAME"
