# External Libraries

A mirror of all external library sources is available in a [requester-pays](https://docs.aws.amazon.com/AmazonS3/latest/userguide/RequesterPaysBuckets.html) S3 bucket named `tmbasic`. Use the [AWSCLI](https://aws.amazon.com/cli/) command `aws s3 cp s3://tmbasic/____ . --request-payer` to download a mirrored file.

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
- Source: https://github.com/arximboldi/immer/archive/4d1caac17daaea58b949e30c6b1d5d5b88a3b78e.zip
    - Mirror: `s3://tmbasic/immer/immer-4d1caac17daaea58b949e30c6b1d5d5b88a3b78e.zip`

### [libstdc++](https://gcc.gnu.org/onlinedocs/libstdc++/)
- License: [GNU General Public License v3](https://github.com/electroly/tmbasic/blob/master/ext/gcc/GPL-3) with [GCC Runtime Library Exception v3.1](https://github.com/electroly/tmbasic/blob/master/ext/gcc/copyright)
- Source (Linux): [`libstdc++` package from Alpine Linux](https://pkgs.alpinelinux.org/packages?name=libstdc%2B%2B&branch=edge)
- Source (Windows): [`mingw-w64-gcc` package from Arch Linux](https://archlinux.org/packages/community/x86_64/mingw-w64-gcc/)

### [mpdecimal](https://www.bytereef.org/mpdecimal/)
- License: [BSD license](https://github.com/electroly/tmbasic/blob/master/ext/mpdecimal/LICENSE.txt)
- Source: https://www.bytereef.org/software/mpdecimal/releases/mpdecimal-2.5.1.tar.gz
    - Mirror: `s3://tmbasic/mpdecimal/mpdecimal-2.5.1.tar.gz`

### [musl](https://musl.libc.org/)
- License: [MIT license](https://github.com/electroly/tmbasic/blob/master/ext/musl/COPYRIGHT)
- Source (Linux): [`musl-dev` package from Alpine Linux](https://pkgs.alpinelinux.org/packages?name=musl-dev)

### [ncurses](https://invisible-island.net/ncurses/)
- License: [Ncurses license](https://github.com/electroly/tmbasic/blob/master/ext/ncurses/COPYING)
- Source: `ftp://ftp.invisible-island.net/ncurses/ncurses-6.2.tar.gz`
    - Mirror: `s3://tmbasic/ncurses/ncurses-6.2.tar.gz`

### [tvision](https://github.com/magiblot/tvision)
- License: [Borland license (original code), MIT license (additions)](https://github.com/electroly/tmbasic/blob/master/ext/tvision/COPYRIGHT)
- Source: https://github.com/magiblot/tvision/archive/3a364725214fe2475e8bbe2ca09c1080b29e3a0f.zip
    - Mirror: `s3://tmbasic/tvision/tvision-3a364725214fe2475e8bbe2ca09c1080b29e3a0f.zip`

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
