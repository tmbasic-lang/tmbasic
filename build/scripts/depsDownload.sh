#!/bin/bash
# Inputs: $USE_R2_MIRROR (optional)
# run from the build/ directory
set -euo pipefail
mkdir -p downloads
cd downloads

# we will build up a list of filenames here
DEPFILES=

function downloadFile {
    # $1=filename, $2=url

    # If $USE_R2_MIRROR is set, we will download from our S3 bucket instead of the original source.
    if [ -n "${USE_R2_MIRROR:-}" ]; then
        if [ ! -f "$1" ]; then
            echo "Downloading $2 from R2 mirror..."
            URL="https://tmbasic-archive.com/deps/$1"
            curl -L -o "$1.tmp" "$URL"
            mv -f "$1.tmp" "$1"
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
ABSEIL_VERSION=310e6f4f0f202da13720fdd6cb0095e10a98fe1c
BINUTILS_VERSION=2.44
CLI11_VERSION=2.5.0
CMAKE_VERSION=3.31.6
FMT_VERSION=11.1.4
GOOGLETEST_VERSION=1.16.0
IMMER_VERSION=0.8.1
LIBZIP_VERSION=1.11.3
LIEF_VERSION=0.16.4
MICROTAR_VERSION=27076e1b9290e9c7842bb7890a54fcf172406c84
MPDECIMAL_VERSION=4.0.0
NAMEOF_VERSION=0.10.4
NCURSES_VERSION=6.5
TURBO_VERSION=f4d6c6c8def15e2757f344a149fdb4607414f169
TVISION_VERSION=d9bc00957285caf9144ca3d219d8256e34f24170
TZDB_VERSION=2025a
UTF8PROC_VERSION=2.10.0
ZLIB_VERSION=1.3.1

# https://github.com/abseil/abseil-cpp
downloadFile "abseil-$ABSEIL_VERSION.tar.gz" "https://github.com/abseil/abseil-cpp/archive/$ABSEIL_VERSION.tar.gz"

# https://ftp.gnu.org/gnu/binutils
downloadFile "binutils-$BINUTILS_VERSION.tar.gz" "https://ftp.gnu.org/gnu/binutils/binutils-$BINUTILS_VERSION.tar.gz"

# https://github.com/CLIUtils/CLI11/releases
downloadFile "cli11-$CLI11_VERSION.hpp" "https://github.com/CLIUtils/CLI11/releases/download/v$CLI11_VERSION/CLI11.hpp"

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

# https://github.com/arximboldi/immer
downloadFile "immer-$IMMER_VERSION.tar.gz" "https://github.com/arximboldi/immer/archive/refs/tags/v$IMMER_VERSION.tar.gz"

# https://github.com/nih-at/libzip/releases
downloadFile "libzip-$LIBZIP_VERSION.tar.gz" "https://github.com/nih-at/libzip/releases/download/v$LIBZIP_VERSION/libzip-$LIBZIP_VERSION.tar.gz"

# https://github.com/lief-project/LIEF/releases
downloadFile "lief-$LIEF_VERSION.tar.gz" "https://github.com/lief-project/LIEF/archive/refs/tags/$LIEF_VERSION.tar.gz"

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

# https://www.iana.org/time-zones
downloadFile "tzdata-$TZDB_VERSION.tar.gz" "https://data.iana.org/time-zones/releases/tzdata$TZDB_VERSION.tar.gz"
downloadFile "tzcode-$TZDB_VERSION.tar.gz" "https://data.iana.org/time-zones/releases/tzcode$TZDB_VERSION.tar.gz"

# https://github.com/JuliaStrings/utf8proc
downloadFile "utf8proc-$UTF8PROC_VERSION.tar.gz" "https://github.com/JuliaStrings/utf8proc/archive/refs/tags/v$UTF8PROC_VERSION.tar.gz"

# https://zlib.net
downloadFile "zlib-$ZLIB_VERSION.tar.gz" "https://zlib.net/zlib-$ZLIB_VERSION.tar.gz"

rm -f ../files/deps.tar
tar cf ../files/deps.tar $DEPFILES
