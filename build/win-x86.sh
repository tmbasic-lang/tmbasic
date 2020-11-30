#!/bin/bash
set -euxo pipefail

export IMAGE_NAME="tmbasic-win-x86"
export HOST_UID=$(id -u "$USER")
export HOST_GID=$(id -g "$USER")
export ARCH="i686"

cat files/Dockerfile.build-win | envsubst | docker build -t $IMAGE_NAME files -f-

pushd ..
docker run --rm --tty --interactive --volume "$PWD:/code" --workdir /code --name $IMAGE_NAME $IMAGE_NAME
popd
