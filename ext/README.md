# External Libraries

A mirror of external library sources is available in a [requester-pays](https://docs.aws.amazon.com/AmazonS3/latest/userguide/RequesterPaysBuckets.html) S3 bucket named `tmbasic`. Use the [AWSCLI](https://aws.amazon.com/cli/) to access:

- List folders: `aws s3 ls s3://tmbasic/ --request-payer`
- List files in a folder: `aws s3 ls s3://tmbasic/____/ --request-payer`
- Download a file: `aws s3 cp s3://tmbasic/____/____.tar.gz . --request-payer`

## Libraries included in both TMBASIC and user programs

`(L)` - Linux build.
`(W)` - Windows build.

Component | Version | License
-- | -- | --
[boost](https://www.boost.org/) | 1.76.0 | [BSL-1.0](https://github.com/electroly/tmbasic/blob/master/ext/boost/LICENSE_1_0.txt)
[fmt](https://github.com/fmtlib/fmt) | 7.1.3 | [MIT](https://github.com/electroly/tmbasic/blob/master/ext/fmt/LICENSE.rst)
[icu](http://site.icu-project.org/) | 69.1 | [ICU](https://github.com/electroly/tmbasic/blob/master/ext/icu/LICENSE)
[immer](https://github.com/arximboldi/immer) | 39f8690 | [BSL-1.0](https://github.com/electroly/tmbasic/blob/master/ext/immer/LICENSE)
[libclipboard](https://github.com/jtanx/libclipboard) | 1.1 | [MIT](https://github.com/electroly/tmbasic/blob/master/ext/libclipboard/LICENSE)
[libstdc++](https://gcc.gnu.org/onlinedocs/libstdc++/) | 10.2.1_pre1-r3&nbsp;(L)<br>10.3.0&nbsp;(W) | [GPL-3.0-only](https://github.com/electroly/tmbasic/blob/master/ext/gcc/GPL-3) WITH [GCC-exception-3.1](https://github.com/electroly/tmbasic/blob/master/ext/gcc/copyright)
[libXau](https://gitlab.freedesktop.org/xorg/lib/libxau) | 1.0.9 (L) | [MIT-open-group](https://github.com/electroly/tmbasic/blob/master/ext/libXau/COPYING)
[libxcb](https://xcb.freedesktop.org/) | 1.14 (L) | [X11](https://github.com/electroly/tmbasic/blob/master/ext/libxcb/COPYING)
[mpdecimal](https://www.bytereef.org/mpdecimal/) | 2.5.1 | [BSD-2-Clause](https://github.com/electroly/tmbasic/blob/master/ext/mpdecimal/LICENSE.txt)
[musl](https://musl.libc.org/) | 1.2.2-r0 (L) | [MIT](https://github.com/electroly/tmbasic/blob/master/ext/musl/COPYRIGHT)
[ncurses](https://invisible-island.net/ncurses/) | 6.2 | [X11](https://github.com/electroly/tmbasic/blob/master/ext/ncurses/COPYING)
[turbo](https://github.com/magiblot/turbo) | 506457a | [MIT](https://github.com/electroly/tmbasic/blob/master/ext/turbo/COPYRIGHT)
[tvision](https://github.com/magiblot/tvision) | dd5da21 | [Borland, MIT](https://github.com/electroly/tmbasic/blob/master/ext/tvision/COPYRIGHT)
[scintilla](https://www.scintilla.org/) | Forked | [HPND](https://github.com/electroly/tmbasic/blob/master/ext/scintilla/License.txt)

## Libraries included only in TMBASIC

Component | Version | License
-- | -- | --
[bsdiff](https://github.com/mendsley/bsdiff) | b817e94 | [BSD-2-Clause](https://github.com/electroly/tmbasic/blob/master/ext/bsdiff/LICENSE)
[bzip2](https://gitlab.com/federicomenaquintero/bzip2) | 6211b650 | [bzip2-1.0.6](https://github.com/electroly/tmbasic/blob/master/ext/bzip2/COPYING)
[nameof](https://github.com/Neargye/nameof) | a9813bd | [MIT](https://github.com/electroly/tmbasic/blob/master/ext/nameof/LICENSE.txt)
