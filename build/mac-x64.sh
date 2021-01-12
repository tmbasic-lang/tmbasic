#!/bin/bash
set -euxo pipefail

cd ..
mkdir -p mac
cd mac

if [ ! -d "cmake" ]
then
    curl -L -o cmake.tar.gz https://github.com/Kitware/CMake/releases/download/v3.18.4/cmake-3.18.4-Darwin-x86_64.tar.gz
    tar zxf cmake.tar.gz
    mv cmake-* cmake
fi

if [ ! -d "boost" ]
then
    curl -L -o boost.tar.gz https://dl.bintray.com/boostorg/release/1.74.0/source/boost_1_74_0.tar.gz
    tar zxf boost.tar.gz
    mv boost_* boost
fi

if [ ! -d "mpdecimal" ]
then
    curl -L -o mpdecimal.tar.gz http://www.bytereef.org/software/mpdecimal/releases/mpdecimal-2.5.0.tar.gz
    tar zxf mpdecimal.tar.gz
    mv mpdecimal-* mpdecimal
    pushd mpdecimal
    ./configure
    make
    popd
fi

if [ ! -d "googletest" ]
then
    curl -L -o googletest.zip https://github.com/google/googletest/archive/release-1.10.0.zip
    unzip googletest.zip
    mv googletest-* googletest
    pushd googletest
    mkdir build
    cd build
    ../../cmake/CMake.app/Contents/bin/cmake ..
    make -j8
    popd
fi

if [ ! -d "immer" ]
then
    curl -L -o immer.zip https://github.com/arximboldi/immer/archive/800ddb04e528a3e83e69e8021d7e872e7c34cbcd.zip
    unzip -q immer.zip
    mv immer-*/ immer/
fi

if [ ! -d "tvision" ]
then
    curl -L -o tvision.zip https://github.com/magiblot/tvision/archive/ccf1841f7ea3fc4ca0cb00f30a3ef49c60d5ff12.zip
    unzip tvision.zip
    mv tvision-* tvision
    pushd tvision
    mkdir build
    cd build
    ../../cmake/CMake.app/Contents/bin/cmake -D CMAKE_PREFIX_PATH=../../ncurses ..
    make -j8
    popd
fi

cd ..
TARGET_OS=mac ARCH=x86_64 make help
TARGET_OS=mac ARCH=x86_64 PS1="[tmbasic-mac-x64] \w\$ " MAKEFLAGS="-j8" bash "$@"
