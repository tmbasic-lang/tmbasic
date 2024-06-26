#!/bin/bash
# Optional parameters: $IMAGE_NAME, $CONTAINER_NAME, $NO_BUILD, $PUSH_ONLY, $HOST_UID, $HOST_GID
set -euxo pipefail

export IMAGE_NAME=${IMAGE_NAME:="tmbasic-linux-arm32"}
export CONTAINER_NAME=${CONTAINER_NAME:="tmbasic-linux-arm32"}
export HOST_UID=${HOST_UID:=$(id -u "$USER")}
export HOST_GID=${HOST_GID:=$(id -g "$USER")}
export DOCKER_ARCH="arm32v7"
export ARCH="arm32v7"
export TRIPLE="armv7-alpine-linux-musleabihf"

# Build unless $NO_BUILD is non-empty.
if [ -z "${NO_BUILD+x}" ]; then
    scripts/sysrootDownload.sh
    scripts/depsDownload.sh

    # If $PUSH_ONLY is non-empty, then set the --push argument.
    export BUILDX_FLAGS=""
    if [ -n "${PUSH_ONLY+x}" ]; then
        export BUILDX_FLAGS="--push"
    fi

    if [ "$(docker image ls $IMAGE_NAME | wc -l)" == "1" ]; then
        cat files/Dockerfile.build-linux | sed "s=\$IMAGE_NAME=$IMAGE_NAME=g; s/\$DOCKER_ARCH/$DOCKER_ARCH/g; s/\$ARCH/$ARCH/g; s/\$USER/$USER/g; s/\$TRIPLE/$TRIPLE/g" | docker buildx build --progress plain $BUILDX_FLAGS -t $IMAGE_NAME files -f-
    fi
fi

# If $PUSH_ONLY is empty or unset, then run.
if [ -z "${PUSH_ONLY+x}" ]; then
    cd ..
    docker run --rm ${TTY_FLAG:=--tty --interactive} --volume "$PWD:/code" --workdir /code --name $CONTAINER_NAME --user $HOST_UID ${DOCKER_FLAGS:= } $IMAGE_NAME "$@"
fi
