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
    curl -L -o boost.tar.gz https://dl.bintray.com/boostorg/release/1.75.0/source/boost_1_75_0.tar.gz
    tar zxf boost.tar.gz
    mv boost_* boost
fi

if [ ! -d "mpdecimal" ]
then
    curl -L -o mpdecimal.tar.gz https://www.bytereef.org/software/mpdecimal/releases/mpdecimal-2.5.1.tar.gz
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
    curl -L -o immer.zip https://github.com/arximboldi/immer/archive/4d1caac17daaea58b949e30c6b1d5d5b88a3b78e.zip
    unzip -q immer.zip
    mv immer-*/ immer/
fi

if [ ! -d "tvision" ]
then
    curl -L -o tvision.zip https://github.com/magiblot/tvision/archive/3a364725214fe2475e8bbe2ca09c1080b29e3a0f.zip
    unzip tvision.zip
    mv tvision-* tvision
    pushd tvision
    mkdir build
    cd build
    ../../cmake/CMake.app/Contents/bin/cmake -D CMAKE_PREFIX_PATH=../../ncurses -DCMAKE_BUILD_TYPE=Release ..
    make -j8
    popd
fi

if [ ! -d "bzip2" ]
then
    curl -L -o bzip2.zip https://gitlab.com/federicomenaquintero/bzip2/-/archive/6211b6500c8bec13a17707440d3a84ca8b34eed5/bzip2-6211b6500c8bec13a17707440d3a84ca8b34eed5.zip
    unzip -q bzip2.zip
    mv bzip2-*/ bzip2/
    mkdir -p bzip2/build
    pushd bzip2/build
    ../../cmake/CMake.app/Contents/bin/cmake .. -DENABLE_STATIC_LIB=ON -DCMAKE_BUILD_TYPE=Release
    make -j8
    popd
fi

if [ ! -d "bsdiff" ]
then
    curl -L -o bsdiff.zip https://github.com/mendsley/bsdiff/archive/b817e9491cf7b8699c8462ef9e2657ca4ccd7667.zip
    unzip -q bsdiff.zip
    mv bsdiff-*/ bsdiff/
    pushd bsdiff
    gcc -o bsdiff -isystem ../bzip2 bsdiff.c -DBSDIFF_EXECUTABLE -L../bzip2/build -lbz2_static
    gcc -o bspatch -isystem ../bzip2 bspatch.c -DBSPATCH_EXECUTABLE -L../bzip2/build -lbz2_static
    gcc -c bsdiff.c
    ar rcs libbsdiff.a bsdiff.o
    gcc -c bspatch.c
    ar rcs libbspatch.a bspatch.o
    popd
fi

if [ ! -d "icu" ]
then
    curl -L -o icu.tar.gz https://github.com/unicode-org/icu/releases/download/release-68-2/icu4c-68_2-src.tgz
    tar zxf icu.tar.gz
    pushd icu/source
    CXXFLAGS="-DU_USING_ICU_NAMESPACE=0 -DU_NO_DEFAULT_INCLUDE_UTF_HEADERS=1 -DU_HIDE_OBSOLETE_UTF_OLD_H=1 -std=c++17" \
        ./runConfigureICU "MacOSX" --enable-static --disable-shared --disable-tests --disable-samples \
        --with-data-packaging=static
    make -j8
    popd
fi

set +x

cd ..
TARGET_OS=mac ARCH=x86_64 make help
TARGET_OS=mac ARCH=x86_64 PS1="[tmbasic-mac-x64] \w\$ " MAKEFLAGS="-j8" bash "$@"
