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

set +x
cd ..
make help
MAC_INCLUDE_FLAGS="-I$(PWD)/mac/boost -I$(PWD)/mac/mpdecimal/libmpdec -I$(PWD)/mac/mpdecimal/libmpdec++ -I$(PWD)/mac/ncurses/include -I$(PWD)/mac/googletest/googletest/include" \
    MAC_LD_FLAGS="-L$(PWD)/mac/mpdecimal/libmpdec -L$(PWD)/mac/mpdecimal/libmpdec++" \
    CMAKE="$(PWD)/mac/cmake/CMake.app/Contents/bin/cmake -D CMAKE_PREFIX_PATH=$(PWD)/mac/ncurses" \
    HELP_FILE_OBJ="" \
    MAC_HELP_FILE_LINK_FLAG="-Wl,-sectcreate,__DATA,__help_h32,obj/help.h32" \
    STATIC_FLAG="" \
    LIBTINFO_FLAG="" \
    LIBMPDEC_FLAG="$(PWD)/mac/mpdecimal/libmpdec/libmpdec.a $(PWD)/mac/mpdecimal/libmpdec++/libmpdec++.a " \
    LIBNCURSESW_FLAG="-lncurses" \
    LIBGTEST_FLAG="$(PWD)/mac/googletest/build/lib/libgtest.a $(PWD)/mac/googletest/build/lib/libgtest_main.a" \
    TARGET_OS=mac \
    PS1="[tmbasic-mac-x64] \w\$ " \
    MAKEFLAGS="-j8" \
    bash
