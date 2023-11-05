#!/bin/bash
# arguments: $BUILDX_ARCH $IMAGE_ARCH $IMAGE_NAME
set -euo pipefail

echo "Generating sysroot for $BUILDX_ARCH ($IMAGE_NAME)"

cat Dockerfile.sysroot \
    | envsubst \
    | docker buildx build --push --platform "$BUILDX_ARCH" -t "$IMAGE_NAME" . -f- \
    ;

echo "Pushed: $IMAGE_NAME"
