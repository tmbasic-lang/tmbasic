#!/bin/bash
set -euo pipefail

export IMAGE_NAME="tmbasic-win-x64"
export HOST_UID=$(id -u "$USER")
export HOST_GID=$(id -g "$USER")
export ARCH="x86_64"
export LIB3264="--disable-lib32 --enable-lib64"

if [ $(uname -m) == 'x86_64' ]; then
export WINE_INSTALL="RUN dpkg --add-architecture i386 ; apt-get update -y ; apt-get install -y wine32 wine64"
else
export WINE_INSTALL="RUN echo Not installing wine on this platform."
fi

files/depsDownload.sh

if [ "$(docker image ls $IMAGE_NAME | wc -l)" == "1" ]; then
    cat files/Dockerfile.build-win | sed "s/\$IMAGE_NAME/$IMAGE_NAME/g; s/\$HOST_UID/$HOST_UID/g; s/\$HOST_GID/$HOST_GID/g; s/\$ARCH/$ARCH/g; s/\$LIB3264/$LIB3264/g; s/\$WINE_INSTALL/$WINE_INSTALL/g" | docker buildx build -t $IMAGE_NAME files -f-
fi

cd ..
docker run --rm ${TTY_FLAG:=--tty --interactive} --volume "$PWD:/code" --workdir /code --name $IMAGE_NAME ${DOCKER_FLAGS:= } $IMAGE_NAME "$@"
