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

mkdir -p bin
mkdir -p lib
mkdir -p man
mkdir -p share
mkdir -p include
mkdir -p pkgs
mkdir -p tmp
export PREFIX="$PWD"
export PKGDIR="$PWD/pkgs"
export PATH=$PREFIX/bin:$PATH

cd tmp

export CMAKE_VERSION=3.20.1
if [ ! -f "$PKGDIR/cmake-$CMAKE_VERSION" ]; then
    rm -rf cmake-*/
    curl -L https://github.com/Kitware/CMake/releases/download/v$CMAKE_VERSION/cmake-$CMAKE_VERSION-macos-universal.tar.gz | tar zx
    pushd cmake-*/
    cp -rf CMake.app/Contents/bin/* "$PREFIX/bin/"
    cp -rf CMake.app/Contents/doc/* "$PREFIX/doc/"
    cp -rf CMake.app/Contents/man/* "$PREFIX/man/"
    cp -rf CMake.app/Contents/share/* "$PREFIX/share/"
    popd
    rm -rf cmake-*/
    touch "$PKGDIR/cmake-$CMAKE_VERSION"
fi

export BOOST_VERSION=1.75.0
if [ ! -f "$PKGDIR/boost-$BOOST_VERSION" ]
then
    rm -rf boost_*/
    curl -L https://dl.bintray.com/boostorg/release/$BOOST_VERSION/source/boost_$(echo $BOOST_VERSION | tr '.' '_').tar.gz | tar zx
    pushd boost_*/
    mv -f boost "$PREFIX/include/"
    popd
    rm -rf boost_*/
    touch "$PKGDIR/boost-$BOOST_VERSION"
fi

export MPDECIMAL_VERSION=2.5.1
if [ ! -f "$PKGDIR/mpdecimal-$MPDECIMAL_VERSION" ]
then
    rm -rf mpdecimal-*/
    curl -L https://www.bytereef.org/software/mpdecimal/releases/mpdecimal-$MPDECIMAL_VERSION.tar.gz | tar zx
    pushd mpdecimal-*/
    curl -L -o config.guess 'https://git.savannah.gnu.org/gitweb/?p=config.git;a=blob_plain;f=config.guess;hb=HEAD'
    curl -L -o config.sub 'https://git.savannah.gnu.org/gitweb/?p=config.git;a=blob_plain;f=config.sub;hb=HEAD'
    CC="clang -arch $MACARCH -mmacosx-version-min=$MACVER" \
        CXX="clang++ -arch $MACARCH -mmacosx-version-min=$MACVER" \
        ./configure --host=$MACTRIPLE "--prefix=$PREFIX"
    make -j8
    make install
    popd
    rm -rf mpdecimal-*/
    touch "$PKGDIR/mpdecimal-$MPDECIMAL_VERSION"
fi

export GOOGLETEST_VERSION=1.10.0
if [ ! -f "$PKGDIR/googletest-$GOOGLETEST_VERSION" ]
then
    rm -rf googletest-*/
    curl -L https://github.com/google/googletest/archive/release-$GOOGLETEST_VERSION.tar.gz | tar zx
    pushd googletest-*/
    mkdir build
    cd build
    cmake .. \
        -DCMAKE_OSX_ARCHITECTURES="$MACARCH" \
        -DCMAKE_OSX_DEPLOYMENT_TARGET="$MACVER" \
        -DCMAKE_INSTALL_PREFIX="$PREFIX"
    make -j8
    make install
    popd
    rm -rf googletest-*/
    touch "$PKGDIR/googletest-$GOOGLETEST_VERSION"
fi

export IMMER_VERSION=4d1caac17daaea58b949e30c6b1d5d5b88a3b78e
if [ ! -f "$PKGDIR/immer-$IMMER_VERSION" ]
then
    rm -rf immer-*/
    curl -L https://github.com/arximboldi/immer/archive/$IMMER_VERSION.tar.gz | tar zx
    pushd immer-*/
    cp -rf immer "$PREFIX/include/"
    popd
    rm -rf immer-*/
    touch "$PKGDIR/immer-$IMMER_VERSION"
fi

export TVISION_VERSION=ded9eba5f873f61976be737951a74d68da8dc942
if [ ! -f "$PKGDIR/tvision-$TVISION_VERSION" ]
then
    rm -rf tvision-*/
    curl -L https://github.com/magiblot/tvision/archive/$TVISION_VERSION.tar.gz | tar zx
    pushd tvision-*/
    mkdir build
    cd build
    CXXFLAGS="-DTVISION_STL=1 -D__cpp_lib_string_view=1" \
        cmake .. \
        -DCMAKE_INSTALL_PREFIX="$PREFIX" \
        -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_OSX_ARCHITECTURES="$MACARCH" \
        -DCMAKE_OSX_DEPLOYMENT_TARGET="$MACVER"
    make -j8
    make install
    popd
    rm -rf tvision-*/
    touch "$PKGDIR/tvision-$TVISION_VERSION"
fi

export BZIP2_VERSION=6211b6500c8bec13a17707440d3a84ca8b34eed5
if [ ! -f "$PKGDIR/bzip2-$BZIP2_VERSION" ]
then
    rm -rf bzip2-*/
    curl -L https://gitlab.com/federicomenaquintero/bzip2/-/archive/$BZIP2_VERSION/bzip2-$BZIP2_VERSION.tar.gz | tar zx
    pushd bzip2-*/
    mkdir build
    cd build
    cmake .. \
        -DCMAKE_INSTALL_PREFIX="$PREFIX" \
        -DENABLE_STATIC_LIB=ON \
        -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_OSX_ARCHITECTURES="$MACARCH" \
        -DCMAKE_OSX_DEPLOYMENT_TARGET="$MACVER"
    make -j8
    make install
    popd
    rm -rf bzip2-*/
    touch "$PKGDIR/bzip2-$BZIP2_VERSION"
fi

export BSDIFF_VERSION=b817e9491cf7b8699c8462ef9e2657ca4ccd7667
if [ ! -f "$PKGDIR/bsdiff-$BSDIFF_VERSION" ]
then
    rm -rf bsdiff-*/
    curl -L https://github.com/mendsley/bsdiff/archive/$BSDIFF_VERSION.tar.gz | tar zx
    pushd bsdiff-*/
    clang -o "$PREFIX/bin/bsdiff" -arch $MACARCH -mmacosx-version-min=$MACVER -isystem "$PREFIX/include" bsdiff.c \
        -DBSDIFF_EXECUTABLE -L"$PREFIX/lib" -lbz2_static
    clang -o "$PREFIX/bin/bspatch" -arch $MACARCH -mmacosx-version-min=$MACVER -isystem "$PREFIX/include" bspatch.c \
        -DBSPATCH_EXECUTABLE -L"$PREFIX/lib" -lbz2_static
    clang -arch $MACARCH -mmacosx-version-min=$MACVER -c bsdiff.c
    ar rcs "$PREFIX/lib/libbsdiff.a" bsdiff.o
    clang -arch $MACARCH -mmacosx-version-min=$MACVER -c bspatch.c
    ar rcs "$PREFIX/lib/libbspatch.a" bspatch.o
    popd
    rm -rf bsdiff-*/
    touch "$PKGDIR/bsdiff-$BSDIFF_VERSION"
fi

export ICU_VERSION=69.1
if [ ! -f "$PKGDIR/icu-$ICU_VERSION" ]
then
    rm -rf icu/
    curl -L https://github.com/unicode-org/icu/releases/download/release-$(echo $ICU_VERSION | tr '.' '-')/icu4c-$(echo $ICU_VERSION | tr '.' '_')-src.tgz | tar zx
    pushd icu/source
    curl -L -o config.guess 'https://git.savannah.gnu.org/gitweb/?p=config.git;a=blob_plain;f=config.guess;hb=HEAD'
    curl -L -o config.sub 'https://git.savannah.gnu.org/gitweb/?p=config.git;a=blob_plain;f=config.sub;hb=HEAD'
    CFLAGS="-arch $MACARCH -mmacosx-version-min=$MACVER" \
        CXXFLAGS="-arch $MACARCH -mmacosx-version-min=$MACVER -DU_USING_ICU_NAMESPACE=0 -DU_NO_DEFAULT_INCLUDE_UTF_HEADERS=1 -DU_HIDE_OBSOLETE_UTF_OLD_H=1 -std=c++17" \
        ./runConfigureICU "MacOSX" --enable-static --disable-shared --disable-tests --disable-samples \
        --with-data-packaging=static --host=$MACTRIPLE "--prefix=$PREFIX"
    cp -f config/mh-darwin config/mh-unknown
    gnumake -j8
    make install
    popd
    rm -rf icu/
    touch "$PKGDIR/icu-$ICU_VERSION"
fi

export FMT_VERSION=7.1.3
if [ ! -f "$PKGDIR/fmt-$FMT_VERSION" ]
then
    rm -rf fmt-*/ fmt.zip
    curl -L -o fmt.zip https://github.com/fmtlib/fmt/releases/download/$FMT_VERSION/fmt-$FMT_VERSION.zip
    unzip -q fmt.zip
    pushd fmt-*/
    mkdir build
    cd build
    cmake .. \
        -DCMAKE_INSTALL_PREFIX="$PREFIX" \
        -DCMAKE_BUILD_TYPE=Release \
        -DFMT_TEST=OFF \
        -DFMT_FUZZ=OFF \
        -DFMT_CUDA_TEST=OFF \
        -DFMT_DOC=OFF \
        -DCMAKE_OSX_ARCHITECTURES="$MACARCH" \
        -DCMAKE_OSX_DEPLOYMENT_TARGET="$MACVER"
    make -j8
    make install
    popd
    rm -rf fmt.zip fmt-*/
    touch "$PKGDIR/fmt-$FMT_VERSION"
fi

export LIBCLIPBOARD_VERSION=1.1
if [ ! -f "$PKGDIR/libclipboard-$LIBCLIPBOARD_VERSION" ]
then
    rm -rf libclipboard-*/
    curl -L https://github.com/jtanx/libclipboard/archive/refs/tags/v$LIBCLIPBOARD_VERSION.tar.gz | tar zx
    pushd libclipboard-*/
    mkdir build
    cd build
    cmake .. \
        -DCMAKE_INSTALL_PREFIX="$PREFIX" \
        -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_OSX_ARCHITECTURES="$MACARCH" \
        -DCMAKE_OSX_DEPLOYMENT_TARGET="$MACVER"
    make -j8
    make install
    popd
    rm -rf libclipboard-*/
    touch "$PKGDIR/libclipboard-$LIBCLIPBOARD_VERSION"
fi

export TURBO_VERSION=defc734d2621052806a4fa3510a91a8453895208
if [ ! -f "$PKGDIR/turbo-$TURBO_VERSION" ]
then
    rm -rf turbo-*/
    curl -L https://github.com/magiblot/turbo/archive/$TURBO_VERSION.tar.gz | tar zx
    pushd turbo-*/
    mv scintilla/lexers/LexBasic.cxx .
    rm -f scintilla/lexers/*
    mv -f LexBasic.cxx scintilla/lexers/LexBasic.cxx
    cat scintilla/src/Catalogue.cxx | sed 's:LINK_LEXER(lm.*::g; s:return 1;:LINK_LEXER(lmFreeBasic); return 1;:g' > Catalogue.cxx
    mv -f Catalogue.cxx scintilla/src/Catalogue.cxx
    mkdir build
    cd build
    cmake .. \
        -DCMAKE_INSTALL_PREFIX="$PREFIX" \
        -DCMAKE_PREFIX_PATH="$PREFIX" \
        -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_OSX_ARCHITECTURES="$MACARCH" \
        -DCMAKE_OSX_DEPLOYMENT_TARGET="$MACVER" \
        -DTURBO_USE_SYSTEM_TVISION=ON \
        -DTURBO_USE_SYSTEM_DEPS=ON \
        -DCMAKE_EXE_LINKER_FLAGS="-framework ServiceManagement -framework Foundation -framework Cocoa"
    make -j8
    cp -f *.a "$PREFIX/lib/"
    cp -f turbo "$PREFIX/bin/"
    mkdir -p "$PREFIX/include/turbo/"
    cp -f $(find ../ -name '*.h') "$PREFIX/include/turbo/"
    popd
    rm -rf turbo-*/
    touch "$PKGDIR/turbo-$TURBO_VERSION"
fi

rm -f $PREFIX/lib/*.dylib

set +x

cd ../../

TARGET_OS=mac ARCH=$DOCKER_ARCH make help
TARGET_OS=mac ARCH=$DOCKER_ARCH PS1="[tmbasic-mac-$SHORT_ARCH] \w\$ " MAKEFLAGS="-j8" BASH_SILENCE_DEPRECATION_WARNING=1 bash "$@"
