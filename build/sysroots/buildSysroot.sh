#!/bin/bash
# arguments: $DOCKER_ARCH
set -euo pipefail
TIMESTAMP=$(date --utc "+%Y%m%d%H%M%S")
FILENAME=sysroot-$(echo $DOCKER_ARCH | sed s:/::g)-$TIMESTAMP.tar.gz

echo "--------------------------------------------------------------------------------"
echo "Generating sysroot for $DOCKER_ARCH..."
cat Dockerfile.sysroot | envsubst | docker buildx build --platform "linux/$DOCKER_ARCH" -t "sysroot" . -f-
docker create --platform "linux/$DOCKER_ARCH" --name sysroot sysroot
docker export sysroot > sysroot-temp.tar
docker container rm sysroot
docker image rm sysroot
gzip -1 sysroot-temp.tar
mv sysroot-temp.tar.gz $FILENAME
aws s3 cp $FILENAME s3://tmbasic/linux-sysroots/$FILENAME --acl public-read
rm $FILENAME
echo "--------------------------------------------------------------------------------"
echo
