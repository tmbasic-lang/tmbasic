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
    curl -L -o tvision.zip https://github.com/magiblot/tvision/archive/df4e5e6e2af1896af2d49050c63886d84d57c27a.zip
    unzip tvision.zip
    mv tvision-* tvision
    pushd tvision
    mkdir build
    cd build
    ../../cmake/CMake.app/Contents/bin/cmake -D CMAKE_PREFIX_PATH=../../ncurses ..
    make -j8
    popd
fi

set +x
cd ..
make help
MAC_INCLUDE_FLAGS="-I$(PWD)/mac/boost -I$(PWD)/mac/mpdecimal/libmpdec -I$(PWD)/mac/mpdecimal/libmpdec++ -I$(PWD)/mac/ncurses/include -I$(PWD)/mac/googletest/googletest/include -I$(PWD)/mac/tvision/include -I$(PWD)/mac/immer" \
    BUILDDOC_MAC_INCLUDE_FLAGS="-I$(PWD)/mac/boost" \
    MAC_LD_FLAGS="-L$(PWD)/mac/mpdecimal/libmpdec -L$(PWD)/mac/mpdecimal/libmpdec++ -L$(PWD)/mac/tvision/build" \
    LINUX_RESOURCE_OBJ_FILES="" \
    MAC_RESOURCES_LINK_FLAGS="-Wl,-sectcreate,__DATA,__help_h32,obj/help.h32 -Wl,-sectcreate,__DATA,__runner_linux_arm32,obj/runner_linux_arm32 -Wl,-sectcreate,__DATA,__runner_linux_arm64,obj/runner_linux_arm64 -Wl,-sectcreate,__DATA,__runner_linux_x64,obj/runner_linux_x64 -Wl,-sectcreate,__DATA,__runner_linux_x86,obj/runner_linux_x86 -Wl,-sectcreate,__DATA,__runner_mac_x64,obj/runner_mac_x64 -Wl,-sectcreate,__DATA,__runner_win_x64,obj/runner_win_x64 -Wl,-sectcreate,__DATA,__runner_win_x86,obj/runner_win_x86" \
    STATIC_FLAG="" \
    LIBTINFO_FLAG="" \
    LIBMPDEC_FLAG="$(PWD)/mac/mpdecimal/libmpdec/libmpdec.a $(PWD)/mac/mpdecimal/libmpdec++/libmpdec++.a " \
    LIBNCURSESW_FLAG="-lncurses" \
    LIBGTEST_FLAG="$(PWD)/mac/googletest/build/lib/libgtest.a $(PWD)/mac/googletest/build/lib/libgtest_main.a" \
    TARGET_OS=mac \
    PS1="[tmbasic-mac-x64] \w\$ " \
    MAKEFLAGS="-j8" \
    TVHC="$(PWD)/mac/tvision/build/tvhc" \
    bash
