# External Libraries

A mirror of external library sources is available in a [requester-pays](https://docs.aws.amazon.com/AmazonS3/latest/userguide/RequesterPaysBuckets.html) S3 bucket named `tmbasic`. Use the [AWSCLI](https://aws.amazon.com/cli/) to access:

- List files in a folder: `aws s3 ls s3://tmbasic/____/ --request-payer`
- Download a file: `aws s3 cp s3://tmbasic/____/____.tar.gz . --request-payer`

## Libraries included in both TMBASIC and user programs

`(L)` - Linux build.
`(W)` - Windows build.

Component | Version | License | Source | Mirror directory
-- | -- | -- | -- | --
[boost](https://www.boost.org/) | 1.75.0 | [BSL-1.0](https://github.com/electroly/tmbasic/blob/master/ext/boost/LICENSE_1_0.txt) | [boost_1_75_0.tar.gz](https://dl.bintray.com/boostorg/release/1.75.0/source/boost_1_75_0.tar.gz) | `s3://tmbasic/boost/`
[fmt](https://github.com/fmtlib/fmt) | 7.1.3 | [MIT](https://github.com/electroly/tmbasic/blob/master/ext/fmt/LICENSE.rst) | [fmt-7.1.3.zip](https://github.com/fmtlib/fmt/releases/download/7.1.3/fmt-7.1.3.zip) | `s3://tmbasic/fmt/`
[icu](http://site.icu-project.org/) | 68.2 | [ICU](https://github.com/electroly/tmbasic/blob/master/ext/icu/LICENSE) | [icu4c-68_2-src.tgz](https://github.com/unicode-org/icu/releases/download/release-68-2/icu4c-68_2-src.tgz) | `s3://tmbasic/icu/`
[immer](https://github.com/arximboldi/immer) | 4d1caac | [BSL-1.0](https://github.com/electroly/tmbasic/blob/master/ext/immer/LICENSE) | [immer-4d1caac...&nbsp;.zip](https://github.com/arximboldi/immer/archive/4d1caac17daaea58b949e30c6b1d5d5b88a3b78e.zip) | `s3://tmbasic/immer/`
[libclipboard](https://github.com/jtanx/libclipboard) | 1.1 | [MIT](https://github.com/electroly/tmbasic/blob/master/ext/libclipboard/LICENSE) | [libclipboard-v1.1.zip](https://github.com/jtanx/libclipboard/archive/refs/tags/v1.1.zip) | `s3://tmbasic/libclipboard/`
[libstdc++](https://gcc.gnu.org/onlinedocs/libstdc++/) | 10.2.1_pre1-r3&nbsp;(L)<br>10.2.0-2&nbsp;(W) | [GPL-3.0-only](https://github.com/electroly/tmbasic/blob/master/ext/gcc/GPL-3)<br>WITH [GCC-exception-3.1](https://github.com/electroly/tmbasic/blob/master/ext/gcc/copyright) | [Alpine/libstdc++&nbsp;(L)](https://pkgs.alpinelinux.org/packages?name=libstdc%2B%2B&branch=edge)<br>[Arch/mingw-w64-gcc&nbsp;(W)](https://archlinux.org/packages/community/x86_64/mingw-w64-gcc/) | &mdash;
[libXau](https://gitlab.freedesktop.org/xorg/lib/libxau) | 1.0.9 (L) | [MIT-open-group](https://github.com/electroly/tmbasic/blob/master/ext/libXau/COPYING) | [libXau-1.0.9.tar.gz](https://xorg.freedesktop.org/archive/individual/lib/libXau-1.0.9.tar.gz) | `s3://tmbasic/libXau/`
[libxcb](https://xcb.freedesktop.org/) | 1.14 (L) | [X11](https://github.com/electroly/tmbasic/blob/master/ext/libxcb/COPYING) | [libxcb-1.14.tar.gz](https://xorg.freedesktop.org/archive/individual/lib/libxcb-1.14.tar.gz) | `s3://tmbasic/libxcb/`
[mpdecimal](https://www.bytereef.org/mpdecimal/) | 2.5.1 | [BSD-2-Clause](https://github.com/electroly/tmbasic/blob/master/ext/mpdecimal/LICENSE.txt) | [mpdecimal-2.5.1.tar.gz](https://www.bytereef.org/software/mpdecimal/releases/mpdecimal-2.5.1.tar.gz) | `s3://tmbasic/mpdecimal/`
[musl](https://musl.libc.org/) | 1.2.2-r0 (L) | [MIT](https://github.com/electroly/tmbasic/blob/master/ext/musl/COPYRIGHT) | [Alpine/musl-dev](https://pkgs.alpinelinux.org/packages?name=musl-dev) | &mdash;
[ncurses](https://invisible-island.net/ncurses/) | 6.2 | [X11](https://github.com/electroly/tmbasic/blob/master/ext/ncurses/COPYING) | [ncurses-6.2.tar.gz](https://invisible-mirror.net/archives/ncurses/ncurses-6.2.tar.gz) | `s3://tmbasic/ncurses/`
[turbo](https://github.com/magiblot/turbo) | defc734 | [MIT](https://github.com/electroly/tmbasic/blob/master/ext/turbo/COPYRIGHT) | [turbo-defc734...&nbsp;.zip](https://github.com/magiblot/turbo/archive/defc734d2621052806a4fa3510a91a8453895208.zip) | `s3://tmbasic/turbo/`
[tvision](https://github.com/magiblot/tvision) | 6aeac84 | [Borland (original)<br>MIT (additions)](https://github.com/electroly/tmbasic/blob/master/ext/tvision/COPYRIGHT) | [tvision-6aeac84...&nbsp;.zip](https://github.com/magiblot/tvision/archive/6aeac843a1d21205461c13a2c536d3c4d012e053.zip) | `s3://tmbasic/tvision/`
[scintilla](https://www.scintilla.org/) | Forked | [HPND](https://github.com/electroly/tmbasic/blob/master/ext/scintilla/License.txt) | (included in `turbo`) | &mdash;

## Libraries included only in TMBASIC

Component | Version | License | Source | Mirror directory
-- | -- | -- | -- | --
[bsdiff](https://github.com/mendsley/bsdiff) | b817e94 | [BSD-2-Clause](https://github.com/electroly/tmbasic/blob/master/ext/bsdiff/LICENSE) | [bsdiff-b817e94...&nbsp;.zip](https://github.com/mendsley/bsdiff/archive/b817e9491cf7b8699c8462ef9e2657ca4ccd7667.zip) | `s3://tmbasic/bsdiff/`
[bzip2](https://gitlab.com/federicomenaquintero/bzip2) | 6211b650 | [bzip2-1.0.6](https://github.com/electroly/tmbasic/blob/master/ext/bzip2/COPYING) | [bzip2-6211b650...&nbsp;.zip](https://gitlab.com/federicomenaquintero/bzip2/-/archive/6211b6500c8bec13a17707440d3a84ca8b34eed5/bzip2-6211b6500c8bec13a17707440d3a84ca8b34eed5.zip) | `s3://tmbasic/bzip2/`
[nameof](https://github.com/Neargye/nameof) | 0.10.0 | [MIT](https://github.com/electroly/tmbasic/blob/master/ext/nameof/LICENSE.txt) | [nameof.hpp](https://github.com/Neargye/nameof/releases/download/v0.10.0/nameof.hpp) | (included in this repository)
