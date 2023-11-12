#!/bin/bash
# Inputs: $USE_S3_MIRROR (optional)
# run from the build/ directory
set -euo pipefail
mkdir -p downloads
cd downloads

# we will build up a list of filenames here
DEPFILES=

function downloadFile {
    # $1=filename, $2=url

    # If $USE_S3MIRROR is set, we will download from our S3 bucket instead of the original source.
    if [ -n "${USE_S3_MIRROR:-}" ]; then
        if [ ! -f "$1" ]; then
            echo "Downloading $2 from S3 mirror..."
            aws s3 cp "s3://tmbasic/deps/$1" "$1" --request-payer requester
        fi
        DEPFILES="$DEPFILES $1"
        return
    fi

    # Otherwise, download from the original source.
    if [ ! -f "$1" ]; then
        echo "Downloading $2..."
        curl -L -o "$1.tmp" --retry 5 "$2"
        mv -f "$1.tmp" "$1"
    fi
    DEPFILES="$DEPFILES $1"
}

# don't update these versions by hand. instead, run scripts/depsCheck.sh
BINUTILS_VERSION=2.37
BOOST_VERSION=1.83.0
CMAKE_VERSION=3.27.7
FMT_VERSION=10.1.1
GOOGLETEST_VERSION=1.14.0
ICU_VERSION=74.1
IMMER_VERSION=0.8.1
LIBZIP_VERSION=1.10.1
MICROTAR_VERSION=27076e1b9290e9c7842bb7890a54fcf172406c84
MPDECIMAL_VERSION=2.5.1
NAMEOF_VERSION=0.10.3
NCURSES_VERSION=6.4
TURBO_VERSION=5d7e794b511d0b4e2257e567f7df9f79d5f43be6
TVISION_VERSION=e9fac379992815aaadcba99f55d8d34b7037a311
ZLIB_VERSION=1.3

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
downloadFile "googletest-$GOOGLETEST_VERSION.tar.gz" "https://github.com/google/googletest/archive/refs/tags/v$GOOGLETEST_VERSION.tar.gz"

# https://github.com/unicode-org/icu/releases
downloadFile "icu-$ICU_VERSION.tar.gz" "https://github.com/unicode-org/icu/releases/download/release-$(echo $ICU_VERSION | tr '.' '-')/icu4c-$(echo $ICU_VERSION | tr '.' '_')-src.tgz"

# https://github.com/arximboldi/immer
downloadFile "immer-$IMMER_VERSION.tar.gz" "https://github.com/arximboldi/immer/archive/refs/tags/v$IMMER_VERSION.tar.gz"

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

# https://zlib.net
downloadFile "zlib-$ZLIB_VERSION.tar.gz" "https://zlib.net/zlib-$ZLIB_VERSION.tar.gz"

rm -f ../files/deps.tar
tar cf ../files/deps.tar $DEPFILES
