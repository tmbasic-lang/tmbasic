#!/bin/bash
set -euxo pipefail

pushd ..
mkdir -p mac
cd mac

if [ ! -d "cmake" ]
then
    curl -L -o cmake.tar.gz https://github.com/Kitware/CMake/releases/download/v3.18.4/cmake-3.18.4-Darwin-x86_64.tar.gz
    tar zxf cmake.tar.gz
    mv cmake-* cmake
fi

if [ ! -d "node" ]
then
    curl -L -o node.tar.gz https://nodejs.org/dist/v15.0.1/node-v15.0.1-darwin-x64.tar.gz
    tar zxf node.tar.gz
    mv node-* node
fi

if [ ! -d "boost" ]
then
    curl -L -o boost.tar.gz https://dl.bintray.com/boostorg/release/1.74.0/source/boost_1_74_0.tar.gz
    tar zxf boost.tar.gz
    mv boost_* boost
fi

if [ ! -d "mpdecimal" ]
then
    tar zxf ../ext/mpdecimal/mpdecimal-2.5.0.tar.gz
    mv mpdecimal-* mpdecimal
    pushd mpdecimal
    ./configure
    make
    popd
fi

if [ ! -d "ncurses" ]
then
    curl -L -o ncurses.tar.gz https://homebrew.bintray.com/bottles/ncurses-6.2.high_sierra.bottle.tar.gz
    tar zxf ncurses.tar.gz
    pushd ncurses
    mv -f 6.2/* ./
    popd
fi

popd
