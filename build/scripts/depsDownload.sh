#!/bin/bash
# run from the build/ directory
set -euo pipefail
mkdir -p downloads
cd downloads

# we will build up a list of filenames here
DEPFILES=

function downloadFile {
    # $1=filename, $2=url
    if [ ! -f "$1" ]; then
        echo "Downloading $2..."
        curl -L -o "$1.tmp" "$2"
        mv -f "$1.tmp" "$1"
    fi
    DEPFILES="$DEPFILES $1"
}

# don't update these versions by hand. instead, run scripts/depsCheck.sh
BINUTILS_VERSION=2.38
BOOST_VERSION=1.78.0
CMAKE_VERSION=3.23.0
FMT_VERSION=8.1.1
GOOGLETEST_VERSION=1.11.0
ICU_VERSION=70.1
IMMER_VERSION=a1271fa712342f5c6dfad876820da17e10c28214
LIBCLIPBOARD_VERSION=1.1
LIBXAU_VERSION=1.0.9
LIBXCB_VERSION=1.14
LIBZIP_VERSION=1.8.0
MICROTAR_VERSION=27076e1b9290e9c7842bb7890a54fcf172406c84
MPDECIMAL_VERSION=2.5.1
NAMEOF_VERSION=0.10.1
NCURSES_VERSION=6.3
TURBO_VERSION=80d2b67b9bce7eaa49a77ad05c367f5c99ae6afb
TVISION_VERSION=399f4c5ce485f6940d202a1a64f2d755d5d1b422
XCBPROTO_VERSION=70ca65fa35c3760661b090bc4b2601daa7a099b8
XORGPROTO_VERSION=2021.5
ZLIB_VERSION=1.2.12

# https://ftp.gnu.org/gnu/binutils
downloadFile "binutils-$BINUTILS_VERSION.tar.gz" "https://ftp.gnu.org/gnu/binutils/binutils-$BINUTILS_VERSION.tar.gz"

# https://boostorg.jfrog.io/artifactory/main/release/
downloadFile "boost-$BOOST_VERSION.tar.gz" "https://boostorg.jfrog.io/artifactory/main/release/$BOOST_VERSION/source/boost_$(echo $BOOST_VERSION | tr '.' '_').tar.gz"

# https://github.com/Kitware/CMake/releases
downloadFile "cmake-mac-$CMAKE_VERSION.tar.gz" "https://github.com/Kitware/CMake/releases/download/v$CMAKE_VERSION/cmake-$CMAKE_VERSION-macos-universal.tar.gz"
downloadFile "cmake-linux-glibc-aarch64-$CMAKE_VERSION.tar.gz" "https://github.com/Kitware/CMake/releases/download/v$CMAKE_VERSION/cmake-$CMAKE_VERSION-linux-aarch64.tar.gz"
downloadFile "cmake-linux-glibc-x86_64-$CMAKE_VERSION.tar.gz" "https://github.com/Kitware/CMake/releases/download/v$CMAKE_VERSION/cmake-$CMAKE_VERSION-linux-x86_64.tar.gz"

# https://git.savannah.gnu.org/gitweb/?p=config.git
downloadFile "config.guess" "https://git.savannah.gnu.org/gitweb/?p=config.git;a=blob_plain;f=config.guess;hb=HEAD"
downloadFile "config.sub" "https://git.savannah.gnu.org/gitweb/?p=config.git;a=blob_plain;f=config.sub;hb=HEAD"

# https://github.com/fmtlib/fmt/releases
downloadFile "fmt-$FMT_VERSION.tar.gz" "https://github.com/fmtlib/fmt/archive/refs/tags/$FMT_VERSION.tar.gz"

# https://github.com/google/googletest/releases
downloadFile "googletest-$GOOGLETEST_VERSION.tar.gz" "https://github.com/google/googletest/archive/release-$GOOGLETEST_VERSION.tar.gz"

# https://github.com/unicode-org/icu/releases
downloadFile "icu-$ICU_VERSION.tar.gz" "https://github.com/unicode-org/icu/releases/download/release-$(echo $ICU_VERSION | tr '.' '-')/icu4c-$(echo $ICU_VERSION | tr '.' '_')-src.tgz"

# https://github.com/arximboldi/immer
downloadFile "immer-$IMMER_VERSION.tar.gz" "https://github.com/arximboldi/immer/archive/$IMMER_VERSION.tar.gz"

# https://github.com/jtanx/libclipboard/releases
downloadFile "libclipboard-$LIBCLIPBOARD_VERSION.tar.gz" "https://github.com/jtanx/libclipboard/archive/refs/tags/v$LIBCLIPBOARD_VERSION.tar.gz"

# https://xorg.freedesktop.org/archive/individual/lib
downloadFile "libXau-$LIBXAU_VERSION.tar.gz" "https://xorg.freedesktop.org/archive/individual/lib/libXau-$LIBXAU_VERSION.tar.gz"

# https://xorg.freedesktop.org/archive/individual/lib
downloadFile "libxcb-$LIBXCB_VERSION.tar.gz" "https://xorg.freedesktop.org/archive/individual/lib/libxcb-$LIBXCB_VERSION.tar.gz"

# https://github.com/nih-at/libzip/releases
downloadFile "libzip-$LIBZIP_VERSION.tar.gz" "https://github.com/nih-at/libzip/releases/download/v$LIBZIP_VERSION/libzip-$LIBZIP_VERSION.tar.gz"

# https://github.com/rxi/microtar
downloadFile "microtar-$MICROTAR_VERSION.tar.gz" "https://github.com/rxi/microtar/archive/$MICROTAR_VERSION.tar.gz"

# https://www.bytereef.org/mpdecimal/
downloadFile "mpdecimal-$MPDECIMAL_VERSION.tar.gz" "https://www.bytereef.org/software/mpdecimal/releases/mpdecimal-$MPDECIMAL_VERSION.tar.gz"

# https://github.com/Neargye/nameof
downloadFile "nameof-$NAMEOF_VERSION.tar.gz" "https://github.com/Neargye/nameof/archive/refs/tags/v$NAMEOF_VERSION.tar.gz"

# https://invisible-mirror.net/ncurses/announce.html
downloadFile "ncurses-$NCURSES_VERSION.tar.gz" "https://invisible-mirror.net/archives/ncurses/ncurses-$NCURSES_VERSION.tar.gz"

# https://github.com/magiblot/turbo
downloadFile "turbo-$TURBO_VERSION.tar.gz" "https://github.com/magiblot/turbo/archive/$TURBO_VERSION.tar.gz"

# https://github.com/magiblot/tvision
downloadFile "tvision-$TVISION_VERSION.tar.gz" "https://github.com/magiblot/tvision/archive/$TVISION_VERSION.tar.gz"

# https://gitlab.freedesktop.org/xorg/proto/xcbproto
downloadFile "xcbproto-$XCBPROTO_VERSION.tar.gz" "https://gitlab.freedesktop.org/xorg/proto/xcbproto/-/archive/$XCBPROTO_VERSION/xcbproto-$XCBPROTO_VERSION.tar.gz"

# https://xorg.freedesktop.org/archive/individual/proto/
downloadFile "xorgproto-$XORGPROTO_VERSION.tar.gz" "https://xorg.freedesktop.org/archive/individual/proto/xorgproto-$XORGPROTO_VERSION.tar.gz"

# https://zlib.net
downloadFile "zlib-$ZLIB_VERSION.tar.gz" "https://zlib.net/zlib-$ZLIB_VERSION.tar.gz"

rm -f ../files/deps.tar
tar cf ../files/deps.tar $DEPFILES
