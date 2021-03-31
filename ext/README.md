# External Libraries

A mirror of external library sources is available in a [requester-pays](https://docs.aws.amazon.com/AmazonS3/latest/userguide/RequesterPaysBuckets.html) S3 bucket named `tmbasic`. Use the [AWSCLI](https://aws.amazon.com/cli/) to access:

- List files in a folder: `aws s3 ls s3://tmbasic/____/ --request-payer`
- Download a file: `aws s3 cp s3://tmbasic/____/____.tar.gz . --request-payer`

## Libraries included in both TMBASIC and user programs

Component | Version | License | Source | Mirror directory
-- | -- | -- | -- | --
[boost](https://www.boost.org/) | 1.75.0 | [Boost Software License v1.0](https://github.com/electroly/tmbasic/blob/master/ext/boost/LICENSE_1_0.txt) | [boost_1_75_0.tar.gz](https://dl.bintray.com/boostorg/release/1.75.0/source/boost_1_75_0.tar.gz) | `s3://tmbasic/boost/`
[fmt](https://github.com/fmtlib/fmt) | 7.1.3 | [MIT license](https://github.com/electroly/tmbasic/blob/master/ext/fmt/LICENSE.rst) | [fmt-7.1.3.zip](https://github.com/fmtlib/fmt/releases/download/7.1.3/fmt-7.1.3.zip) | `s3://tmbasic/fmt/`
[icu](http://site.icu-project.org/) | 68.2 | [ICU License](https://github.com/electroly/tmbasic/blob/master/ext/icu/LICENSE) | [icu4c-68_2-src.tgz](https://github.com/unicode-org/icu/releases/download/release-68-2/icu4c-68_2-src.tgz) | `s3://tmbasic/icu/`
[immer](https://github.com/arximboldi/immer) | 4d1caac | [Boost Software License v1.0](https://github.com/electroly/tmbasic/blob/master/ext/immer/LICENSE) | [immer-4d1caac... .zip](https://github.com/arximboldi/immer/archive/4d1caac17daaea58b949e30c6b1d5d5b88a3b78e.zip) | `s3://tmbasic/immer/`
[libclipboard](https://github.com/jtanx/libclipboard) | 1.1 | [MIT license](https://github.com/electroly/tmbasic/blob/master/ext/libclipboard/LICENSE) | [libclipboard-v1.1.zip](https://github.com/jtanx/libclipboard/archive/refs/tags/v1.1.zip) | `s3://tmbasic/libclipboard/`
[libstdc++](https://gcc.gnu.org/onlinedocs/libstdc++/) | 10.2.1_pre1-r3 (Linux)<br>? (Windows) | [GNU GPL v3](https://github.com/electroly/tmbasic/blob/master/ext/gcc/GPL-3)<br>[GCC Runtime Library Exception v3.1](https://github.com/electroly/tmbasic/blob/master/ext/gcc/copyright) | [Alpine/`libstdc++` (Linux)](https://pkgs.alpinelinux.org/packages?name=libstdc%2B%2B&branch=edge)<br>[Arch/`mingw-w64-gcc` (Windows)](https://archlinux.org/packages/community/x86_64/mingw-w64-gcc/) | --
[libXau](https://gitlab.freedesktop.org/xorg/lib/libxau) | 1.0.9 | [X11 license](https://github.com/electroly/tmbasic/blob/master/ext/libXau/COPYING) | [libXau-1.0.9.tar.gz](https://xorg.freedesktop.org/archive/individual/lib/libXau-1.0.9.tar.gz) | `s3://tmbasic/libXau/`
[libxcb](https://xcb.freedesktop.org/) | 1.14 | [MIT license](https://github.com/electroly/tmbasic/blob/master/ext/libxcb/COPYING) | [libxcb-1.14.tar.gz](https://xorg.freedesktop.org/archive/individual/lib/libxcb-1.14.tar.gz) | `s3://tmbasic/libxcb/`
[mpdecimal](https://www.bytereef.org/mpdecimal/) | 2.5.1 | [BSD license](https://github.com/electroly/tmbasic/blob/master/ext/mpdecimal/LICENSE.txt) | [mpdecimal-2.5.1.tar.gz](https://www.bytereef.org/software/mpdecimal/releases/mpdecimal-2.5.1.tar.gz) | `s3://tmbasic/mpdecimal/`
[musl](https://musl.libc.org/) | 1.2.2-r0 | [MIT license](https://github.com/electroly/tmbasic/blob/master/ext/musl/COPYRIGHT) | [Alpine/`musl-dev` (Linux)](https://pkgs.alpinelinux.org/packages?name=musl-dev) | --
[ncurses](https://invisible-island.net/ncurses/) | 6.2 | [Ncurses license](https://github.com/electroly/tmbasic/blob/master/ext/ncurses/COPYING) | [ncurses-6.2.tar.gz](https://invisible-mirror.net/archives/ncurses/ncurses-6.2.tar.gz) | `s3://tmbasic/ncurses/`
[turbo](https://github.com/magiblot/turbo) | 8cbf8a9 | [MIT license](https://github.com/electroly/tmbasic/blob/master/ext/turbo/COPYRIGHT) | [turbo-8cbf8a9... .zip](https://github.com/magiblot/turbo/archive/8cbf8a9bc735f2a867761fc5fc5e2e3d49452ec0.zip) | `s3://tmbasic/turbo/`
[tvision](https://github.com/magiblot/tvision) | 3a36472 | [Borland license (original code)<br>MIT license (additions)](https://github.com/electroly/tmbasic/blob/master/ext/tvision/COPYRIGHT) | [tvision-3a36472... .zip](https://github.com/magiblot/tvision/archive/3a364725214fe2475e8bbe2ca09c1080b29e3a0f.zip) | `s3://tmbasic/tvision/`
[scintilla](https://www.scintilla.org/) | Forked | [Scintilla license](https://github.com/electroly/tmbasic/blob/master/ext/scintilla/License.txt) | (included in `turbo`) | --

## Libraries included only in TMBASIC

Component | Version | License | Source | Mirror directory
-- | -- | -- | -- | --
[bsdiff](https://github.com/mendsley/bsdiff) | b817e94 | [BSD license](https://github.com/electroly/tmbasic/blob/master/ext/bsdiff/LICENSE) | [bsdiff-b817e94... .zip](https://github.com/mendsley/bsdiff/archive/b817e9491cf7b8699c8462ef9e2657ca4ccd7667.zip) | `s3://tmbasic/bsdiff/`
[bzip2](https://gitlab.com/federicomenaquintero/bzip2) | 6211b650 | [BSD license](https://github.com/electroly/tmbasic/blob/master/ext/bzip2/COPYING) | [bzip2-6211b650... .zip](https://gitlab.com/federicomenaquintero/bzip2/-/archive/6211b6500c8bec13a17707440d3a84ca8b34eed5/bzip2-6211b6500c8bec13a17707440d3a84ca8b34eed5.zip) | `s3://tmbasic/bzip2/`
[nameof](https://github.com/Neargye/nameof) | 0.10.0 | [MIT license](https://github.com/electroly/tmbasic/blob/master/ext/nameof/LICENSE.txt) | [nameof.hpp](https://github.com/Neargye/nameof/releases/download/v0.10.0/nameof.hpp) | (included in this repository)
