#!/bin/bash
# Optional parameters: $IMAGE_NAME, $CONTAINER_NAME, $NO_BUILD, $BUILD_ONLY, $HOST_UID, $HOST_GID
set -euxo pipefail

export IMAGE_NAME=${IMAGE_NAME:="tmbasic-linux-x86"}
export CONTAINER_NAME=${CONTAINER_NAME:="tmbasic-linux-x86"}
export HOST_UID=${HOST_UID:=$(id -u "$USER")}
export HOST_GID=${HOST_GID:=$(id -g "$USER")}
export DOCKER_ARCH="i386"
export ARCH="i686"
export TRIPLE="i586-alpine-linux-musl"

# Build unless $NO_BUILD is non-empty.
if [ -z "${NO_BUILD+x}" ]; then
    scripts/sysrootDownload.sh
    scripts/depsDownload.sh

    if [ "$(docker image ls $IMAGE_NAME | wc -l)" == "1" ]; then
        cat files/Dockerfile.build-linux | sed "s=\$IMAGE_NAME=$IMAGE_NAME=g; s/\$DOCKER_ARCH/$DOCKER_ARCH/g; s/\$ARCH/$ARCH/g; s/\$USER/$USER/g; s/\$TRIPLE/$TRIPLE/g" | docker buildx build --load --progress plain -t $IMAGE_NAME files -f-
    fi
fi

# If $BUILD_ONLY is empty or unset, then run.
if [ -z "${BUILD_ONLY+x}" ]; then
    cd ..
    docker run --rm ${TTY_FLAG:=--tty --interactive} --volume "$PWD:/code" --workdir /code --name $CONTAINER_NAME --user $HOST_UID ${DOCKER_FLAGS:= } $IMAGE_NAME "$@"
fi
