#!/bin/bash
set -euxo pipefail

# don't update these versions by hand. instead, run scripts/depsCheck.sh
PKG_CONFIG_VERSION=0.29.2
BINUTILS_VERSION=2.37  # build breaks on 2.38
MINGW_VERSION=9.0.0
GCC_VERSION=11.2.0

curl -L https://pkg-config.freedesktop.org/releases/pkg-config-$PKG_CONFIG_VERSION.tar.gz | tar -xz &
curl -L https://ftp.gnu.org/gnu/binutils/binutils-$BINUTILS_VERSION.tar.xz | tar -xJ &
curl -L https://sourceforge.net/projects/mingw-w64/files/mingw-w64/mingw-w64-release/mingw-w64-v$MINGW_VERSION.tar.bz2 | tar -xj &
curl -L https://ftp.gnu.org/gnu/gcc/gcc-$GCC_VERSION/gcc-$GCC_VERSION.tar.xz | tar -xJ &
wait
curl -L https://raw.githubusercontent.com/msys2/MINGW-packages/master/mingw-w64-gcc/0020-libgomp-Don-t-hard-code-MS-printf-attributes.patch \
    | patch -d gcc-$GCC_VERSION -p 1

mkdir -p $MINGW/include $MINGW/lib/pkgconfig
chmod 0777 -R $MINGW
cd pkg-config-$PKG_CONFIG_VERSION
./configure \
    --prefix=/usr \
    --with-pc-path=$MINGW/lib/pkgconfig \
    --with-internal-glib \
    --disable-shared \
    --disable-nls
make -j$(nproc)
make install
cd ..

cd binutils-$BINUTILS_VERSION
./configure \
    --prefix=/usr \
    --target=$ARCH-w64-mingw32 \
    --disable-shared \
    --enable-static \
    --disable-lto \
    --disable-plugins \
    --disable-multilib \
    --disable-nls \
    --disable-werror \
    --with-system-zlib
make -j$(nproc)
make install
cd ..

mkdir mingw-w64
cd mingw-w64
../mingw-w64-v$MINGW_VERSION/mingw-w64-headers/configure \
    --prefix=/usr/$ARCH-w64-mingw32 \
    --host=$ARCH-w64-mingw32 \
    --enable-sdk=all \
    --enable-secure-api
make install
cd ..

mkdir gcc
cd gcc
../gcc-$GCC_VERSION/configure \
    --prefix=/usr \
    --target=$ARCH-w64-mingw32 \
    --enable-languages=c,c++ \
    --disable-shared \
    --enable-static \
    --enable-threads=posix \
    --with-system-zlib \
    --enable-libgomp \
    --enable-libatomic \
    --enable-graphite \
    --disable-libstdcxx-pch \
    --disable-libstdcxx-debug \
    --disable-multilib \
    --disable-lto \
    --disable-nls \
    --disable-werror
make -j$(nproc) all-gcc
make install-gcc
cd ..

cd mingw-w64
../mingw-w64-v$MINGW_VERSION/mingw-w64-crt/configure \
    --prefix=/usr/$ARCH-w64-mingw32 \
    --host=$ARCH-w64-mingw32 \
    --enable-wildcard \
    $LIB3264
make -j$(nproc)
make install
cd ..

cd mingw-w64
../mingw-w64-v$MINGW_VERSION/mingw-w64-libraries/winpthreads/configure \
    --prefix=/usr/$ARCH-w64-mingw32 \
    --host=$ARCH-w64-mingw32 \
    --enable-static \
    --disable-shared
make -j$(nproc)
make install
cd ..

cd gcc
make -j$(nproc)
make install
cd ..

rm -rf \
    pkg-config-$PKG_CONFIG_VERSION \
    binutils-$BINUTILS_VERSION \
    mingw-w64 \
    mingw-w64-v$MINGW_VERSION \
    gcc \
    gcc-$GCC_VERSION
