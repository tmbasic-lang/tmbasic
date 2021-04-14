#!/bin/bash
set -euxo pipefail

# arguments: $MACTRIPLE $MACVER $MACARCH $SHORT_ARCH
# $MACTRIPLE: x86_64-apple-macos10.13 or arm64-apple-macos11
# $MACVER: 10.13 or 11.0
# $MACARCH: x86_64 or arm64
# $SHORT_ARCH: x64 or arm64
# $DOCKER_ARCH: x86_64 or arm64v8
# run from the build directory.

cd ..
mkdir -p mac-$SHORT_ARCH
cd mac-$SHORT_ARCH

if [ ! -d "cmake" ]
then
    curl -L -o cmake.tar.gz https://github.com/Kitware/CMake/releases/download/v3.20.1/cmake-3.20.1-macos-universal.tar.gz
    tar zxf cmake.tar.gz
    mv cmake-* cmake
fi

export CMAKE=../../cmake/CMake.app/Contents/bin/cmake

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
    curl -L -o config.guess 'https://git.savannah.gnu.org/gitweb/?p=config.git;a=blob_plain;f=config.guess;hb=HEAD'
    curl -L -o config.sub 'https://git.savannah.gnu.org/gitweb/?p=config.git;a=blob_plain;f=config.sub;hb=HEAD'
    CC="clang -arch $MACARCH -mmacosx-version-min=$MACVER" \
        CXX="clang++ -arch $MACARCH -mmacosx-version-min=$MACVER" \
        ./configure --host=$MACTRIPLE
    make -j8
    popd
fi

if [ ! -d "googletest" ]
then
    curl -L -o googletest.zip https://github.com/google/googletest/archive/release-1.10.0.zip
    unzip -q googletest.zip
    mv googletest-* googletest
    mkdir googletest/build
    pushd googletest/build
    $CMAKE -DCMAKE_OSX_ARCHITECTURES="$MACARCH" -DCMAKE_OSX_DEPLOYMENT_TARGET="$MACVER" ..
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
    curl -L -o tvision.zip https://github.com/magiblot/tvision/archive/6aeac843a1d21205461c13a2c536d3c4d012e053.zip
    unzip -q tvision.zip
    mv tvision-* tvision
    mkdir -p tvision/build
    pushd tvision/build
    CXXFLAGS="-DTVISION_STL=1 -D__cpp_lib_string_view=1" \
        $CMAKE -D CMAKE_PREFIX_PATH=../../ncurses -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_OSX_ARCHITECTURES="$MACARCH" -DCMAKE_OSX_DEPLOYMENT_TARGET="$MACVER" ..
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
    $CMAKE -DENABLE_STATIC_LIB=ON -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_OSX_ARCHITECTURES="$MACARCH" -DCMAKE_OSX_DEPLOYMENT_TARGET="$MACVER" ..
    make -j8
    popd
fi

if [ ! -d "bsdiff" ]
then
    curl -L -o bsdiff.zip https://github.com/mendsley/bsdiff/archive/b817e9491cf7b8699c8462ef9e2657ca4ccd7667.zip
    unzip -q bsdiff.zip
    mv bsdiff-*/ bsdiff/
    pushd bsdiff
    clang -arch $MACARCH -mmacosx-version-min=$MACVER -o bsdiff -isystem ../bzip2 bsdiff.c -DBSDIFF_EXECUTABLE -L../bzip2/build -lbz2_static
    clang -arch $MACARCH -mmacosx-version-min=$MACVER -o bspatch -isystem ../bzip2 bspatch.c -DBSPATCH_EXECUTABLE -L../bzip2/build -lbz2_static
    clang -arch $MACARCH -mmacosx-version-min=$MACVER -c bsdiff.c
    ar rcs libbsdiff.a bsdiff.o
    clang -arch $MACARCH -mmacosx-version-min=$MACVER -c bspatch.c
    ar rcs libbspatch.a bspatch.o
    popd
fi

if [ ! -d "icu" ]
then
    curl -L -o icu.tar.gz https://github.com/unicode-org/icu/releases/download/release-68-2/icu4c-68_2-src.tgz
    tar zxf icu.tar.gz
    pushd icu/source
    curl -L -o config.guess 'https://git.savannah.gnu.org/gitweb/?p=config.git;a=blob_plain;f=config.guess;hb=HEAD'
    curl -L -o config.sub 'https://git.savannah.gnu.org/gitweb/?p=config.git;a=blob_plain;f=config.sub;hb=HEAD'
    CFLAGS="-arch $MACARCH -mmacosx-version-min=$MACVER" \
        CXXFLAGS="-arch $MACARCH -mmacosx-version-min=$MACVER -DU_USING_ICU_NAMESPACE=0 -DU_NO_DEFAULT_INCLUDE_UTF_HEADERS=1 -DU_HIDE_OBSOLETE_UTF_OLD_H=1 -std=c++17" \
        ./runConfigureICU "MacOSX" --enable-static --disable-shared --disable-tests --disable-samples \
        --with-data-packaging=static --host=$MACTRIPLE
    cp -f config/mh-darwin config/mh-unknown
    gnumake -j8
    popd
fi

if [ ! -d "fmt" ]
then
    curl -L -o fmt.zip https://github.com/fmtlib/fmt/releases/download/7.1.3/fmt-7.1.3.zip
    unzip -q fmt.zip
    mv fmt-*/ fmt/
    mkdir -p fmt/build
    pushd fmt/build
    $CMAKE .. -DCMAKE_BUILD_TYPE=Release -DFMT_TEST=OFF -DFMT_FUZZ=OFF -DFMT_CUDA_TEST=OFF -DFMT_DOC=OFF \
        -DCMAKE_OSX_ARCHITECTURES="$MACARCH" -DCMAKE_OSX_DEPLOYMENT_TARGET="$MACVER"
    make -j8
    popd
fi

if [ ! -d "libclipboard" ]
then
    curl -L -o libclipboard.zip https://github.com/jtanx/libclipboard/archive/refs/tags/v1.1.zip
    unzip -q libclipboard.zip
    mv libclipboard-*/ libclipboard/
    mkdir -p libclipboard/build
    pushd libclipboard/build
    $CMAKE .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_OSX_ARCHITECTURES="$MACARCH" -DCMAKE_OSX_DEPLOYMENT_TARGET="$MACVER"
    make -j8
    popd
fi

if [ ! -d "turbo" ]
then
    curl -L -o turbo.zip https://github.com/magiblot/turbo/archive/defc734d2621052806a4fa3510a91a8453895208.zip
    unzip -q turbo.zip
    mv turbo-*/ turbo/
    pushd turbo
    mv scintilla/lexers/LexBasic.cxx .
    rm -f scintilla/lexers/*
    mv -f LexBasic.cxx scintilla/lexers/LexBasic.cxx
    cat scintilla/src/Catalogue.cxx | sed 's:LINK_LEXER(lm.*::g; s:return 1;:LINK_LEXER(lmFreeBasic); return 1;:g' > Catalogue.cxx
    mv -f Catalogue.cxx scintilla/src/Catalogue.cxx
    cat deps/CMakeLists.txt | sed 's:add_subdirectory:#:g' > deps-CMakeLists.txt
    mv -f deps-CMakeLists.txt deps/CMakeLists.txt
    patch CMakeLists.txt ../../build/files/turbo-CMakeLists.txt.diff
    
    mkdir build
    cd build
    CXXFLAGS="-isystem $(PWD)/../../tvision/include -isystem $(PWD)/../../fmt/include -isystem $(PWD)/../../libclipboard/include -isystem $(PWD)/../../libclipboard/build/include -DTVISION_STL=1 -D__cpp_lib_string_view=1" \
        $CMAKE .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_OSX_ARCHITECTURES="$MACARCH" -DCMAKE_OSX_DEPLOYMENT_TARGET="$MACVER"
    make -j8
    mkdir include
    cp $(find ../ -name '*.h') include/
    popd
fi

set +x

cd ..
TARGET_OS=mac ARCH=$DOCKER_ARCH make help
TARGET_OS=mac ARCH=$DOCKER_ARCH PS1="[tmbasic-mac-$SHORT_ARCH] \w\$ " MAKEFLAGS="-j8" bash "$@"
