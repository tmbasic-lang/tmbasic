#!/bin/bash
set -euxo pipefail

# arguments: $ARCH
# $ARCH: x86_64 or arm64v8
# run from the build directory.

if [ "$ARCH" == "x86_64" ]; then
    export SHORT_ARCH=x64
fi
if [ "$ARCH" == "arm64v8" ]; then
    export SHORT_ARCH=arm64
fi

cd ..

mkdir -p mac-$SHORT_ARCH
cd mac-$SHORT_ARCH
mkdir -p bin
mkdir -p lib
mkdir -p man
mkdir -p share
mkdir -p include
mkdir -p pkgs
mkdir -p tmp
export PREFIX="$PWD"
export PATH=$PREFIX/bin:$PATH

cd tmp
TARGET_OS=mac TARGET_CC=clang TARGET_AR=ar NATIVE_PREFIX="$PREFIX" TARGET_PREFIX="$PREFIX" \
    gnumake -j8 -f ../../build/files/deps.mk
cd ..
rm -rf $PREFIX/lib/*.dylib

set +x

cd ../

pwd
TARGET_OS=mac ARCH=$ARCH make help
TARGET_OS=mac ARCH=$ARCH PS1="[tmbasic-mac-$SHORT_ARCH] \w\$ " MAKEFLAGS="-j8" BASH_SILENCE_DEPRECATION_WARNING=1 bash "$@"
