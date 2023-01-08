# Third Party Libraries

A mirror of third party library sources is available in a [requester-pays](https://docs.aws.amazon.com/AmazonS3/latest/userguide/RequesterPaysBuckets.html) S3 bucket named `tmbasic`. Use the [AWSCLI](https://aws.amazon.com/cli/) to access:

- List files: `aws s3 ls s3://tmbasic/deps/ --request-payer`
- Download a file: `aws s3 cp s3://tmbasic/deps/filename.tar.gz . --request-payer`

## Libraries included in both TMBASIC and user programs

Component | License
-- | --
[boost](https://www.boost.org/) | [BSL-1.0](https://github.com/electroly/tmbasic/blob/master/doc/licenses/boost/LICENSE_1_0.txt)
[fmt](https://github.com/fmtlib/fmt) | [MIT](https://github.com/electroly/tmbasic/blob/master/doc/licenses/fmt/LICENSE.rst)
[icu](http://site.icu-project.org/) | [ICU](https://github.com/electroly/tmbasic/blob/master/doc/licenses/icu/LICENSE)
[immer](https://github.com/arximboldi/immer) | [BSL-1.0](https://github.com/electroly/tmbasic/blob/master/doc/licenses/immer/LICENSE)
[libstdc++](https://gcc.gnu.org/onlinedocs/libstdc++/) | [GPL-3.0-only](https://github.com/electroly/tmbasic/blob/master/doc/licenses/gcc/GPL-3) WITH [GCC-exception-3.1](https://github.com/electroly/tmbasic/blob/master/doc/licenses/gcc/copyright)
[mpdecimal](https://www.bytereef.org/mpdecimal/) | [BSD-2-Clause](https://github.com/electroly/tmbasic/blob/master/doc/licenses/mpdecimal/LICENSE.txt)
[musl](https://musl.libc.org/) | [MIT](https://github.com/electroly/tmbasic/blob/master/doc/licenses/musl/COPYRIGHT)
[ncurses](https://invisible-island.net/ncurses/) | [X11](https://github.com/electroly/tmbasic/blob/master/doc/licenses/ncurses/COPYING)
[turbo](https://github.com/magiblot/turbo) | [MIT](https://github.com/electroly/tmbasic/blob/master/doc/licenses/turbo/COPYRIGHT)
[tvision](https://github.com/magiblot/tvision) | [Borland, MIT](https://github.com/electroly/tmbasic/blob/master/doc/licenses/tvision/COPYRIGHT)
[scintilla](https://www.scintilla.org/) | [HPND](https://github.com/electroly/tmbasic/blob/master/doc/licenses/scintilla/License.txt)

## Libraries included only in TMBASIC

Component | License
-- | --
[libzip](https://github.com/nih-at/libzip) | [BSD-3-Clause](https://github.com/electroly/tmbasic/blob/master/doc/licenses/libzip/LICENSE)
[microtar](https://github.com/rxi/microtar) | [MIT](https://github.com/electroly/tmbasic/blob/master/doc/licenses/microtar/LICENSE)
[nameof](https://github.com/Neargye/nameof) | [MIT](https://github.com/electroly/tmbasic/blob/master/doc/licenses/nameof/LICENSE.txt)
[zlib](https://zlib.net) | [Zlib](https://github.com/electroly/tmbasic/blob/master/doc/licenses/zlib/LICENSE.txt)
