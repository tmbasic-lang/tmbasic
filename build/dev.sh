#!/bin/bash
set -euo pipefail

export IMAGE_NAME="tmbasic-dev"
export HOST_UID=$(id -u "$USER")
export HOST_GID=$(id -g "$USER")
export ARCH=$(uname -m)
export GIT_NAME=$(git config --list | grep '^user\.name=' | awk -F= '{print $2}')
export GIT_EMAIL=$(git config --list | grep '^user\.email=' | awk -F= '{print $2}')

if [ "$ARCH" == "aarch64" ]; then
    export ARCH="arm64v8"
fi
if [ "$ARCH" == "arm64" ]; then
    export ARCH="arm64v8"
fi

if [ "$(docker image ls $IMAGE_NAME | wc -l)" == "1" ]; then
    cat files/Dockerfile.build-dev | sed "s/\$IMAGE_NAME/$IMAGE_NAME/g; s/\$HOST_UID/$HOST_UID/g; s/\$HOST_GID/$HOST_GID/g; s/\$ARCH/$ARCH/g; s/\$USER/$USER/g; s/\$GIT_NAME/$GIT_NAME/g; s/\$GIT_EMAIL/$GIT_EMAIL/g" | docker buildx build -t $IMAGE_NAME files -f-
fi

cd ..
docker run --rm ${TTY_FLAG:=--tty --interactive} --volume "$PWD:/code" --workdir /code --name $IMAGE_NAME -p 5000:5000 $IMAGE_NAME "$@"
