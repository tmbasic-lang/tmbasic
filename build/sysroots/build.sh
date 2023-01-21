#!/bin/bash
# arguments: $BUILDX_ARCH $IMAGE_ARCH
set -euo pipefail
TIMESTAMP=$(date --utc "+%Y%m%d%H%M%S")
REPO="public.ecr.aws/k4r2l7j7/tmbasic-linux-sysroot"
TAG="$IMAGE_ARCH-$TIMESTAMP"

echo "Generating sysroot for $BUILDX_ARCH ($TAG)"

cat Dockerfile.sysroot \
    | envsubst \
    | docker buildx build --platform "$BUILDX_ARCH" -t $REPO:$TAG . -f- \
    ;

docker push $REPO:$TAG

echo "Pushed: $REPO:$TAG"
