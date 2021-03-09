# TMBASIC <wbr><span class="tagline">(Text Mode BASIC)</span>

<!-- See DEVELOPERS.md for instructions on generating this screenshot. -->
<a href="https://tmbasic.com/screenshot.png"><img src="https://tmbasic.com/screenshot.png" alt="Screenshot" class="screenshot"></a>

TMBASIC is a simple programming language for creating console applications that run on Windows, macOS, and Linux. Apps written in TMBASIC can be simple command line tools or sophisticated mouse-driven user interfaces.

[Read the documentation online](https://tmbasic.com/doc.html) or in TMBASIC's integrated help viewer.

- [Cheat Sheet for Experienced Programmers](https://tmbasic.com/cheat.html)
- [BASIC Reference](https://tmbasic.com/ref.html)

## Project status
TMBASIC is under development. Stay tuned!

## Compatibility

<div id="platformSupportTable">

<table><tr><td><img src="https://tmbasic.com/windows-logo.png" width=64 height=64 alt="Windows"><br><strong>Windows 10</strong><br><span class="arch">64-bit &bull; 32-bit</span></td><td><img src="https://tmbasic.com/apple-logo.png" width=64 height=64 alt="macOS"><br><strong>macOS 10.13+</strong><br><span class="arch">Intel</span></td><td><img src="https://tmbasic.com/linux-logo.png" width=64 height=64 alt="Linux"><br><strong>Ubuntu &bull; Raspbian &bull; <a href="https://apps.apple.com/us/app/ish-shell/id1436902243">iSH&nbsp;Shell</a>&nbsp;(iOS) &bull; <a href="https://termux.com/">Termux</a>&nbsp;(Android)</strong><br><span class="arch">64-bit &bull; 32-bit &bull; ARM64 &bull; ARM32</span></td></tr></table>

</div>

TMBASIC supports all major desktop systems. It can run on mobile platforms using the popular terminal apps <a href="https://apps.apple.com/us/app/ish-shell/id1436902243">iSH Shell</a> and <a href="https://termux.com/">Termux</a>. TMBASIC running on any platform can produce single-file executables for any other supported platform. Compiled BASIC programs require no external libraries.

## Motivation

There are many options for developing rich graphical user interfaces (GUIs) today, both on the web and on the desktop. Text user interfaces (TUIs) are an older technology, so why consider writing a TUI-based app using TMBASIC today?

**Text mode apps are easy to write.** Developing a high-quality GUI is time-consuming and requires significant expertise. TUIs are inherently more restricted and have fewer options for visual effects than GUIs, and as a result the skill ceiling is lower. A novice can produce TUIs that are just as good as those produced by experts.

**Text mode apps are easy to publish.** Deploying web apps on servers is a complicated matter, requiring experience in server operations. Similarly, distributing GUI toolkits like Qt or Electron requires shipping a lot of moving parts. TMBASIC apps are standalone executable files that can be simply copied to a user's computer. No additional files are required.

**Text mode apps are timeless.** As modern interface conventions continue to evolve, web and desktop apps look outdated after just a few years. A text user interface will not look any more dated in the future than it does today.

**Text mode apps do not need per-platform customizations.** Windows, macOS, and Linux each have their own user interface styles and conventions. A high-quality GUI must follow these conventions or risk looking out of place. The same is not true of TUIs, where expectations are the same on all platforms.

**TMBASIC makes it easy to get started.** Everything you need to develop and distribute sophisticated apps is included in TMBASIC. No other software is required. The BASIC language will feel familiar to anyone with programming experience and is easy to learn for new programmers.

## License
TMBASIC itself is open source software, but apps written in TMBASIC are not required to be open source. The following components are included in builds of user programs.

- tmbasic — _[MIT license](LICENSE)_
- [boost](https://www.boost.org/) — _[Boost Software License v1.0](https://github.com/electroly/tmbasic/blob/master/ext/boost/LICENSE_1_0.txt)_
- [icu](http://site.icu-project.org/) — _[ICU License](https://github.com/electroly/tmbasic/blob/master/ext/icu/LICENSE)_
- [immer](https://github.com/arximboldi/immer) — _[Boost Software License v1.0](https://github.com/electroly/tmbasic/blob/master/ext/immer/LICENSE)_
- [libstdc++](https://gcc.gnu.org/onlinedocs/libstdc++/) — _[GNU General Public License v3](https://github.com/electroly/tmbasic/blob/master/ext/gcc/GPL-3) with [GCC Runtime Library Exception v3.1](https://github.com/electroly/tmbasic/blob/master/ext/gcc/copyright)_
- [mpdecimal](https://www.bytereef.org/mpdecimal/) — _[BSD license](https://github.com/electroly/tmbasic/blob/master/ext/mpdecimal/LICENSE.txt)_
- [musl](https://musl.libc.org/) — _[MIT license](https://github.com/electroly/tmbasic/blob/master/ext/musl/COPYRIGHT)_
- [ncurses](https://en.wikipedia.org/wiki/Ncurses) — _[Ncurses license](https://github.com/electroly/tmbasic/blob/master/ext/ncurses/COPYING)_
- [tvision](https://github.com/magiblot/tvision) — _[Borland license (original code), MIT license (additions)](https://github.com/electroly/tmbasic/blob/master/ext/tvision/COPYRIGHT)_

Pay close attention: executables built by TMBASIC are subject to the licenses above. You must obey these licenses if you distribute your compiled executables. These are permissive open source licenses that do not require you to release your BASIC source code. TMBASIC includes a `LICENSE.txt` file that combines these licenses for your convenience. Include this license file along with your executable when you distribute your TMBASIC apps.

Read the [External Libraries](https://github.com/electroly/tmbasic/blob/master/ext/README.md) document for a list of third party components used in TMBASIC itself.

The Windows, macOS, and Linux logo icons used on the TMBASIC website were created by [Erik Ragnar Eliasson](https://www.iconfinder.com/Erik_Rgnr).
