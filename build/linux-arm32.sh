#!/bin/bash
set -euo pipefail

export IMAGE_NAME="tmbasic-linux-arm32"
export HOST_UID=$(id -u "$USER")
export HOST_GID=$(id -g "$USER")
export DOCKER_ARCH="arm32v7"
export ARCH="arm32v7"
export TRIPLE="armv7-alpine-linux-musleabihf"

scripts/sysrootDownload.sh
scripts/depsDownload.sh

if [ "$(docker image ls $IMAGE_NAME | wc -l)" == "1" ]; then
    cat files/Dockerfile.build-linux | sed "s/\$IMAGE_NAME/$IMAGE_NAME/g; s/\$HOST_UID/$HOST_UID/g; s/\$HOST_GID/$HOST_GID/g; s/\$DOCKER_ARCH/$DOCKER_ARCH/g; s/\$ARCH/$ARCH/g; s/\$USER/$USER/g; s/\$TRIPLE/$TRIPLE/g" | docker buildx build -t $IMAGE_NAME files -f-
fi

cd ..
docker run --rm ${TTY_FLAG:=--tty --interactive} --volume "$PWD:/code" --workdir /code --name $IMAGE_NAME ${DOCKER_FLAGS:= } $IMAGE_NAME "$@"
