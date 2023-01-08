#!/bin/sh
# inputs: $ARCH
# run from the build/ directory
set -eu

if [ "$ARCH" = "arm32v7" ]; then
    export SYSROOT_VERSION="arm32v7-20230108041738"
fi
if [ "$ARCH" = "arm64v8" ]; then
    export SYSROOT_VERSION="arm64v8-20230108042227"
fi
if [ "$ARCH" = "x86_64" ]; then
    export SYSROOT_VERSION="amd64-20230108042628"
fi
if [ "$ARCH" = "i686" ]; then
    export SYSROOT_VERSION="i386-20230108042000"
fi

mkdir -p downloads
if [ ! -f "downloads/sysroot-$SYSROOT_VERSION.tar.gz" ]; then
    echo "Downloading s3://tmbasic/linux-sysroots/sysroot-$SYSROOT_VERSION.tar.gz..."
    aws s3 cp s3://tmbasic/linux-sysroots/sysroot-$SYSROOT_VERSION.tar.gz "downloads/sysroot-$SYSROOT_VERSION.tar.gz" --request-payer
fi
cp -f "downloads/sysroot-$SYSROOT_VERSION.tar.gz" "files/sysroot-$ARCH.tar.gz"
