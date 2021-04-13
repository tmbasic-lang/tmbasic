#!/bin/bash
set -euxo pipefail

export IMAGE_NAME="tmbasic-dev"
export HOST_UID=$(id -u "$USER")
export HOST_GID=$(id -g "$USER")
export ARCH=$(uname -m)

if [ "$ARCH" == "aarch64" ]; then
    export ARCH="arm64v8"
fi
if [ "$ARCH" == "arm64" ]; then
    export ARCH="arm64v8"
fi

cat files/Dockerfile.build-dev | sed "s/\$IMAGE_NAME/$IMAGE_NAME/g; s/\$HOST_UID/$HOST_UID/g; s/\$HOST_GID/$HOST_GID/g; s/\$ARCH/$ARCH/g; s/\$USER/$USER/g" | docker buildx build -t $IMAGE_NAME files -f-

pushd ..
docker run --rm ${TTY_FLAG:=--tty --interactive} --volume "$PWD:/code" --workdir /code --name $IMAGE_NAME -p 5000:5000 $IMAGE_NAME "$@"
popd
