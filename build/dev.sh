#!/bin/bash
set -euxo pipefail

export IMAGE_NAME="tmbasic-dev"
export HOST_UID=$(id -u "$USER")
export HOST_GID=$(id -g "$USER")

cat files/Dockerfile.build-dev | envsubst | docker build -t $IMAGE_NAME files -f-

pushd ..
docker run --rm ${TTY_FLAG:=--tty --interactive} --volume "$PWD:/code" --workdir /code --name $IMAGE_NAME -p 5000:5000 $IMAGE_NAME "$@"
popd
