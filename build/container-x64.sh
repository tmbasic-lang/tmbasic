#!/bin/bash
set -euxo pipefail

export IMAGE_NAME="tmbasic-build-x64"
export HOST_UID=$(id -u "$USER")
export HOST_GID=$(id -g "$USER")
export BASE_IMAGE_NAME="ubuntu:20.04"

if [ -z $(docker images -q $IMAGE_NAME) ]; then
    cat docker/Dockerfile.build | envsubst | docker build -t $IMAGE_NAME docker -f-
fi

pushd ..
docker run --rm --tty --interactive --volume "$PWD:/code" --workdir /code $IMAGE_NAME
popd
