# External Libraries

A mirror of all external library sources is stored in a requester-pays S3 bucket.

## Libraries included in both TMBASIC and user programs

### [boost](https://www.boost.org/)
- License: [Boost Software License v1.0](https://github.com/electroly/tmbasic/blob/master/ext/boost/LICENSE_1_0.txt)
- Source: https://dl.bintray.com/boostorg/release/1.75.0/source/boost_1_75_0.tar.gz
    - Mirror: `s3://tmbasic/boost/boost_1_75_0.tar.gz`

### [icu](http://site.icu-project.org/)
- License: [ICU License](https://github.com/electroly/tmbasic/blob/master/ext/icu/LICENSE)
- Source: https://github.com/unicode-org/icu/releases/download/release-68-2/icu4c-68_2-src.tgz
    - Mirror: `s3://tmbasic/icu/icu4c-68_2-src.tgz`

### [immer](https://github.com/arximboldi/immer)
- License: [Boost Software License v1.0](https://github.com/electroly/tmbasic/blob/master/ext/immer/LICENSE)
- Source: https://github.com/arximboldi/immer/archive/800ddb04e528a3e83e69e8021d7e872e7c34cbcd.zip
    - Mirror: `s3://tmbasic/immer/immer-800ddb04e528a3e83e69e8021d7e872e7c34cbcd.zip`

### [libstdc++](https://gcc.gnu.org/onlinedocs/libstdc++/)
- License: [GNU General Public License v3](https://github.com/electroly/tmbasic/blob/master/ext/gcc/GPL-3) with [GCC Runtime Library Exception v3.1](https://github.com/electroly/tmbasic/blob/master/ext/gcc/copyright)
- Source (Linux): `libstdc++` package from Alpine Linux
- Source (Windows): `mingw-w64-gcc` package from Arch Linux

### [mpdecimal](https://www.bytereef.org/mpdecimal/)
- License: [BSD license](https://github.com/electroly/tmbasic/blob/master/ext/mpdecimal/LICENSE.txt)
- Source: https://www.bytereef.org/software/mpdecimal/releases/mpdecimal-2.5.1.tar.gz
    - Mirror: `s3://tmbasic/mpdecimal/mpdecimal-2.5.1.tar.gz`

### [musl](https://musl.libc.org/)
- License: [MIT license](https://github.com/electroly/tmbasic/blob/master/ext/musl/COPYRIGHT)
- Source (Linux): `musl-dev` package from Alpine Linux

### [ncurses](https://invisible-island.net/ncurses/)
- License: [Ncurses license](https://github.com/electroly/tmbasic/blob/master/ext/ncurses/COPYING)
- Source (Linux): `ncurses` package from Alpine Linux
- Source (Windows): `ftp://ftp.invisible-island.net/ncurses/ncurses-6.2.tar.gz`
    - Mirror: `s3://tmbasic/ncurses/ncurses-6.2.tar.gz`
- Source (macOS): Xcode package

### [tvision](https://github.com/magiblot/tvision)
- License: [Borland license (original code), MIT license (additions)](https://github.com/electroly/tmbasic/blob/master/ext/tvision/COPYRIGHT)
- Source: https://github.com/magiblot/tvision/archive/761d813bf6e0317296545be77a6142198f33ace3.zip
    - Mirror: `s3://tmbasic/tvision/tvision-761d813bf6e0317296545be77a6142198f33ace3.zip`

## Libraries included only in TMBASIC

### [bsdiff](https://github.com/mendsley/bsdiff)
- License: [BSD license](https://github.com/electroly/tmbasic/blob/master/ext/bsdiff/LICENSE)
- Source: https://github.com/mendsley/bsdiff/archive/b817e9491cf7b8699c8462ef9e2657ca4ccd7667.zip
    - Mirror: `s3://tmbasic/bsdiff/bsdiff-b817e9491cf7b8699c8462ef9e2657ca4ccd7667.zip`

### [bzip2](https://gitlab.com/federicomenaquintero/bzip2)
- License: [BSD license](https://github.com/electroly/tmbasic/blob/master/ext/bzip2/COPYING)
- Source: https://gitlab.com/federicomenaquintero/bzip2/-/archive/6211b6500c8bec13a17707440d3a84ca8b34eed5/bzip2-6211b6500c8bec13a17707440d3a84ca8b34eed5.zip
    - Mirror: `s3://tmbasic/bzip2/bzip2-6211b6500c8bec13a17707440d3a84ca8b34eed5.zip`

### [nameof](https://github.com/Neargye/nameof)
- License: [MIT license](https://github.com/electroly/tmbasic/blob/master/ext/nameof/LICENSE.txt)
- Source: https://github.com/Neargye/nameof/releases/download/v0.10.0/nameof.hpp (included in this repository)
