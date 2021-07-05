#!/bin/bash
# arguments: $BUILDX_ARCH $IMAGE_ARCH
set -euo pipefail
TIMESTAMP=$(date --utc "+%Y%m%d%H%M%S")
FILENAME=sysroot-$IMAGE_ARCH-$TIMESTAMP.tar.gz

echo
echo "--------------------------------------------------------------------------------"
echo "Generating sysroot for $BUILDX_ARCH..."
cat Dockerfile.sysroot | envsubst | docker buildx build --platform "$BUILDX_ARCH" -t sysroot . -f-
docker create --platform "$BUILDX_ARCH" --name sysroot sysroot
docker export sysroot > sysroot-temp.tar
docker container rm sysroot
docker image rm sysroot
gzip -9 sysroot-temp.tar
mv sysroot-temp.tar.gz $FILENAME
aws s3 cp $FILENAME s3://tmbasic/linux-sysroots/$FILENAME --acl public-read
rm $FILENAME
echo "--------------------------------------------------------------------------------"
echo
