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

# https://ftp.gnu.org/gnu/binutils
BINUTILS_VERSION=2.36.1
downloadFile "binutils-$BINUTILS_VERSION.tar.gz" "https://ftp.gnu.org/gnu/binutils/binutils-$BINUTILS_VERSION.tar.gz"

# https://boostorg.jfrog.io/artifactory/main/release/
BOOST_VERSION=1.76.0
downloadFile "boost-$BOOST_VERSION.tar.gz" "https://boostorg.jfrog.io/artifactory/main/release/$BOOST_VERSION/source/boost_$(echo $BOOST_VERSION | tr '.' '_').tar.gz"

# https://github.com/Kitware/CMake/releases
CMAKE_VERSION=3.20.4
downloadFile "cmake-mac-$CMAKE_VERSION.tar.gz" "https://github.com/Kitware/CMake/releases/download/v$CMAKE_VERSION/cmake-$CMAKE_VERSION-macos-universal.tar.gz"
downloadFile "cmake-linux-glibc-aarch64-$CMAKE_VERSION.tar.gz" "https://github.com/Kitware/CMake/releases/download/v$CMAKE_VERSION/cmake-$CMAKE_VERSION-linux-aarch64.tar.gz"
downloadFile "cmake-linux-glibc-x86_64-$CMAKE_VERSION.tar.gz" "https://github.com/Kitware/CMake/releases/download/v$CMAKE_VERSION/cmake-$CMAKE_VERSION-linux-x86_64.tar.gz"

# https://git.savannah.gnu.org/gitweb/?p=config.git
downloadFile "config.guess" "https://git.savannah.gnu.org/gitweb/?p=config.git;a=blob_plain;f=config.guess;hb=HEAD"
downloadFile "config.sub" "https://git.savannah.gnu.org/gitweb/?p=config.git;a=blob_plain;f=config.sub;hb=HEAD"

# https://github.com/fmtlib/fmt/releases
FMT_VERSION=8.0.0
downloadFile "fmt-$FMT_VERSION.tar.gz" "https://github.com/fmtlib/fmt/archive/refs/tags/$FMT_VERSION.tar.gz"

# https://github.com/google/googletest/releases
GOOGLETEST_VERSION=1.11.0
downloadFile "googletest-$GOOGLETEST_VERSION.tar.gz" "https://github.com/google/googletest/archive/release-$GOOGLETEST_VERSION.tar.gz"

# https://github.com/unicode-org/icu/releases
ICU_VERSION=69.1
downloadFile "icu-$ICU_VERSION.tar.gz" "https://github.com/unicode-org/icu/releases/download/release-$(echo $ICU_VERSION | tr '.' '-')/icu4c-$(echo $ICU_VERSION | tr '.' '_')-src.tgz"

# https://github.com/arximboldi/immer
IMMER_VERSION=a11df7243cb516a1aeffc83c31366d7259c79e82
downloadFile "immer-$IMMER_VERSION.tar.gz" "https://github.com/arximboldi/immer/archive/$IMMER_VERSION.tar.gz"

# https://github.com/jtanx/libclipboard/releases
LIBCLIPBOARD_VERSION=1.1
downloadFile "libclipboard-$LIBCLIPBOARD_VERSION.tar.gz" "https://github.com/jtanx/libclipboard/archive/refs/tags/v$LIBCLIPBOARD_VERSION.tar.gz"

# https://xorg.freedesktop.org/archive/individual/lib
LIBXAU_VERSION=1.0.9
downloadFile "libXau-$LIBXAU_VERSION.tar.gz" "https://xorg.freedesktop.org/archive/individual/lib/libXau-$LIBXAU_VERSION.tar.gz"

# https://xorg.freedesktop.org/archive/individual/lib
LIBXCB_VERSION=1.14
downloadFile "libxcb-$LIBXCB_VERSION.tar.gz" "https://xorg.freedesktop.org/archive/individual/lib/libxcb-$LIBXCB_VERSION.tar.gz"

# https://github.com/nih-at/libzip/releases
LIBZIP_VERSION=1.8.0
downloadFile "libzip-$LIBZIP_VERSION.tar.gz" "https://github.com/nih-at/libzip/releases/download/v$LIBZIP_VERSION/libzip-$LIBZIP_VERSION.tar.gz"

# https://github.com/rxi/microtar
MICROTAR_VERSION=27076e1b9290e9c7842bb7890a54fcf172406c84
downloadFile "microtar-$MICROTAR_VERSION.tar.gz" "https://github.com/rxi/microtar/archive/$MICROTAR_VERSION.tar.gz"

# https://www.bytereef.org/mpdecimal/
MPDECIMAL_VERSION=2.5.1
downloadFile "mpdecimal-$MPDECIMAL_VERSION.tar.gz" "https://www.bytereef.org/software/mpdecimal/releases/mpdecimal-$MPDECIMAL_VERSION.tar.gz"

# https://github.com/Neargye/nameof
NAMEOF_VERSION=d69f91daa513585d37b4bc600fb6af8b6d99a073
downloadFile "nameof-$NAMEOF_VERSION.tar.gz" "https://github.com/Neargye/nameof/archive/$NAMEOF_VERSION.tar.gz"

# https://invisible-mirror.net/ncurses/announce.html
NCURSES_VERSION=6.2
downloadFile "ncurses-$NCURSES_VERSION.tar.gz" "https://invisible-mirror.net/archives/ncurses/ncurses-$NCURSES_VERSION.tar.gz"

# https://github.com/magiblot/turbo
TURBO_VERSION=a868d2bfedb77a83e9f991154d002ec99e5a180e
downloadFile "turbo-$TURBO_VERSION.tar.gz" "https://github.com/magiblot/turbo/archive/$TURBO_VERSION.tar.gz"

# https://github.com/magiblot/tvision
TVISION_VERSION=46b1b705144bc0d4c6504b99302a39076147896f
downloadFile "tvision-$TVISION_VERSION.tar.gz" "https://github.com/magiblot/tvision/archive/$TVISION_VERSION.tar.gz"

# https://gitlab.freedesktop.org/xorg/proto/xcbproto
XCBPROTO_VERSION=496e3ce329c3cc9b32af4054c30fa0f306deb007
downloadFile "xcbproto-$XCBPROTO_VERSION.tar.gz" "https://gitlab.freedesktop.org/xorg/proto/xcbproto/-/archive/$XCBPROTO_VERSION/xcbproto-$XCBPROTO_VERSION.tar.gz"

# https://xorg.freedesktop.org/archive/individual/proto/
XORGPROTO_VERSION=2021.4.99.2
downloadFile "xorgproto-$XORGPROTO_VERSION.tar.gz" "https://xorg.freedesktop.org/archive/individual/proto/xorgproto-$XORGPROTO_VERSION.tar.gz"

# https://zlib.net
ZLIB_VERSION=1.2.11
downloadFile "zlib-$ZLIB_VERSION.tar.gz" "https://zlib.net/zlib-$ZLIB_VERSION.tar.gz"

rm -f ../files/deps.tar
tar cf ../files/deps.tar $DEPFILES
