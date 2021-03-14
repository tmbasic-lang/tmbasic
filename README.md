# TMBASIC <wbr><span class="tagline">(Text Mode BASIC)</span>

<!-- See DEVELOPERS.md for instructions on generating this screenshot. -->
<a href="https://tmbasic.com/screenshot.png"><img src="https://tmbasic.com/screenshot.png" alt="Screenshot" class="screenshot"></a>

TMBASIC is a simple programming language for creating console applications that run on Windows, macOS, and Linux.
Apps written in TMBASIC can be simple command line tools or sophisticated mouse-driven user interfaces.
Everything you need is built-in.

[Read the documentation online](https://tmbasic.com/doc.html) or in TMBASIC's integrated help viewer.

- [Cheat Sheet for Experienced Programmers](https://tmbasic.com/cheat.html)
- [BASIC Reference](https://tmbasic.com/ref.html)

## Project status
TMBASIC is under development. Stay tuned!

## Why?

There are many options for developing rich graphical user interfaces (GUIs) today, both on the web and on the desktop. Text user interfaces (TUIs) are an older technology, so why consider writing a TUI-based app using TMBASIC today?

<strong><i>Timeless.</i></strong> As modern interface conventions continue to evolve, web and desktop apps look outdated after just a few years. A text user interface will not look any more dated in the future than it does today.

<strong><i>Easy to write.</i></strong> Developing a high-quality modern GUI is time-consuming and requires significant expertise. TUIs have fewer options for visual effects, so the time and skill required is lower.

<strong><i>Easy to publish.</i></strong> Deploying web apps on servers is complicated. Similarly, desktop toolkits like Qt or Electron require a lot of moving parts. TMBASIC apps are standalone executable files that require no supporting libraries. They can be copied and opened without installation.

<strong><i>Easy cross-platform support.</i></strong> Windows, macOS, and Linux each have their own user interface styles and conventions which high-quality GUI apps must follow. The same is not true of TUIs, where expectations are the same everywhere. One build in TMBASIC produces executables for all supported platforms.

<strong><i>Easy to get started.</i></strong> Everything you need to develop and distribute sophisticated apps is included in TMBASIC. The BASIC language will feel familiar to anyone with programming experience and is easy to learn for new programmers.

## Compatibility

<div id="platformSupportTable">

<table><tr><td><img src="https://tmbasic.com/windows-logo.png" width=64 height=64 alt="Windows"></td><td><strong>Windows</strong></td><td><span class="arch">Windows 7, 8, 10<br>64-bit &bull; 32-bit</span></td></tr><tr><td><img src="https://tmbasic.com/apple-logo.png" width=64 height=64 alt="macOS"></td><td><strong>Mac</strong></td><td><span class="arch">macOS 10.13+<br>Intel</span></td><tr><td><img src="https://tmbasic.com/linux-logo.png" width=64 height=64 alt="Linux"></td><td><strong>Linux</strong></td><td><span class="arch">Ubuntu &bull; Raspbian &bull; <a href="https://apps.apple.com/us/app/ish-shell/id1436902243">iSH&nbsp;Shell</a>&nbsp;(iOS) &bull; <a href="https://termux.com/">Termux</a>&nbsp;(Android)<br>64-bit &bull; 32-bit &bull; ARM64 &bull; ARM32</span></td></tr></table>

</div>

TMBASIC supports all major desktop systems. It runs on mobile platforms using the popular terminal apps <a href="https://apps.apple.com/us/app/ish-shell/id1436902243">iSH Shell</a> and <a href="https://termux.com/">Termux</a>. TMBASIC running on any platform can produce single-file executables for any other supported platform. Compiled BASIC programs require no external libraries.

## License
TMBASIC itself is open source software, but apps written in TMBASIC can be closed or open source as desired. The following components are included in builds of user programs.

- tmbasic — [MIT license](LICENSE)
- [boost](https://www.boost.org/) — [Boost Software License v1.0](https://github.com/electroly/tmbasic/blob/master/ext/boost/LICENSE_1_0.txt)
- [icu](http://site.icu-project.org/) — [ICU License](https://github.com/electroly/tmbasic/blob/master/ext/icu/LICENSE)
- [immer](https://github.com/arximboldi/immer) — [Boost Software License v1.0](https://github.com/electroly/tmbasic/blob/master/ext/immer/LICENSE)
- [libstdc++](https://gcc.gnu.org/onlinedocs/libstdc++/) — [GNU General Public License v3](https://github.com/electroly/tmbasic/blob/master/ext/gcc/GPL-3) with [GCC Runtime Library Exception v3.1](https://github.com/electroly/tmbasic/blob/master/ext/gcc/copyright)
- [mpdecimal](https://www.bytereef.org/mpdecimal/) — [BSD license](https://github.com/electroly/tmbasic/blob/master/ext/mpdecimal/LICENSE.txt)
- [musl](https://musl.libc.org/) — [MIT license](https://github.com/electroly/tmbasic/blob/master/ext/musl/COPYRIGHT)
- [ncurses](https://en.wikipedia.org/wiki/Ncurses) — [Ncurses license](https://github.com/electroly/tmbasic/blob/master/ext/ncurses/COPYING)
- [tvision](https://github.com/magiblot/tvision) — [Borland license (original code), MIT license (additions)](https://github.com/electroly/tmbasic/blob/master/ext/tvision/COPYRIGHT)

Executables built by TMBASIC are subject to the licenses above. You must obey these licenses if you distribute your compiled executables. These are permissive open source licenses that do not require you to release your BASIC source code. TMBASIC includes a `LICENSE.txt` file that combines these licenses for your convenience. Include this license file along with your executable when you distribute your TMBASIC apps.

Read the [External Libraries](https://github.com/electroly/tmbasic/blob/master/ext/README.md) document for a list of third party components used in TMBASIC itself.

The Windows, macOS, and Linux logo icons used on the TMBASIC website were created by [Erik Ragnar Eliasson](https://www.iconfinder.com/Erik_Rgnr).
