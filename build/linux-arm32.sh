#!/bin/bash
set -euxo pipefail

export IMAGE_NAME="tmbasic-linux-arm32"
export HOST_UID=$(id -u "$USER")
export HOST_GID=$(id -g "$USER")
export DOCKER_ARCH="arm32v7"
export ARCH="arm32v7"

cat files/Dockerfile.build-linux | envsubst | docker build -t $IMAGE_NAME files -f-

pushd ..
docker run --rm ${TTY_FLAG:=--tty --interactive} --volume "$PWD:/code" --workdir /code --name $IMAGE_NAME ${DOCKER_FLAGS:= } $IMAGE_NAME "$@"
popd
