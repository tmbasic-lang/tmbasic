#!/bin/bash
set -euxo pipefail

export IMAGE_NAME="tmbasic-build-arm64"
export HOST_UID=$(id -u "$USER")
export HOST_GID=$(id -g "$USER")
export BASE_IMAGE_NAME="arm64v8/alpine:3.12"

[ $(uname -m) == "aarch64" ] || docker run --rm --privileged multiarch/qemu-user-static --reset -p yes

cp -f ../ext/mpdecimal/*.tar.gz docker/

cat docker/Dockerfile.build-linux | envsubst | docker build -t $IMAGE_NAME docker -f-

pushd ..
docker run --rm --tty --interactive --volume "$PWD:/code" --workdir /code --name $IMAGE_NAME $IMAGE_NAME
popd
