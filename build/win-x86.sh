#!/bin/bash
# Optional parameters: $IMAGE_NAME, $CONTAINER_NAME, $NO_BUILD, $PUSH_ONLY, $HOST_UID, $HOST_GID
set -euxo pipefail

export IMAGE_NAME=${IMAGE_NAME:="tmbasic-win-x86"}
export CONTAINER_NAME=${CONTAINER_NAME:="tmbasic-win-x86"}
export HOST_UID=${HOST_UID:=$(id -u "$USER")}
export HOST_GID=${HOST_GID:=$(id -g "$USER")}
export ARCH="i686"
export LIB3264="--enable-lib32 --disable-lib64"

if [ $(uname -m) == 'x86_64' ]; then
export WINE_INSTALL="RUN dpkg --add-architecture i386 ; apt-get update -y ; apt-get install -y wine32 wine64"
else
export WINE_INSTALL="RUN echo Not installing wine on this platform."
fi

# Build unless $NO_BUILD is non-empty.
if [ -z "${NO_BUILD+x}" ]; then
    scripts/depsDownload.sh

    # If $PUSH_ONLY is non-empty, then set the --push argument.
    export BUILDX_FLAGS=""
    if [ -n "${PUSH_ONLY+x}" ]; then
        export BUILDX_FLAGS="--push"
    fi

    if [ "$(docker image ls $IMAGE_NAME | wc -l)" == "1" ]; then
        cat files/Dockerfile.build-win | sed "s=\$IMAGE_NAME=$IMAGE_NAME=g; s/\$ARCH/$ARCH/g; s/\$LIB3264/$LIB3264/g; s/\$WINE_INSTALL/$WINE_INSTALL/g" | docker buildx build $BUILDX_FLAGS -t $IMAGE_NAME files -f-
    fi
fi

# If $PUSH_ONLY is empty or unset, then run.
if [ -z "${PUSH_ONLY+x}" ]; then
    cd ..
    docker run --rm ${TTY_FLAG:=--tty --interactive} --volume "$PWD:/code" --workdir /code --name $CONTAINER_NAME --user $HOST_UID ${DOCKER_FLAGS:= } $IMAGE_NAME "$@"
fi
