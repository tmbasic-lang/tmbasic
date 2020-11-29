# TMBASIC

TMBASIC is a simple programming language for creating console applications that run on Windows, macOS, and Linux. "TM" stands for "Text Mode." Apps written in TMBASIC can be simple command line tools or complex mouse-driven user interfaces. A single build in TMBASIC produces executables for all supported platforms with no external libraries and no installation required.

TMBASIC supports the following platforms:

Platform | Tested systems
-- | --
Windows <span class="github-only">(</span><span class="architecture">x64</span><span class="github-only">)</span> | Windows 10 20H2
macOS <span class="github-only">(</span><span class="architecture">x64</span><span class="github-only">)</span> | macOS 10.13 with [iTerm2](https://www.iterm2.com/)
Linux <span class="github-only">(</span><span class="architecture">x64</span><span class="github-only">)</span> | Alpine 3.12, Ubuntu 18.04
Linux <span class="github-only">(</span><span class="architecture">ARM 32</span><span class="github-only">)</span> | Raspberry Pi OS
Linux <span class="github-only">(</span><span class="architecture">ARM 64</span><span class="github-only">)</span> | Alpine 3.12, Ubuntu 18.04, Android 10 with [Termux](https://termux.com/)

[Read the documentation online](https://tmbasic.com/doc.html) or in TMBASIC's integrated help viewer.

- [Cheat sheet for experienced programmers](https://tmbasic.com/cheatSheet.html)
- [BASIC reference](https://tmbasic.com/basic.html)

If you want to build TMBASIC from its C++ source, follow the instructions on the [Developers](https://github.com/electroly/tmbasic/blob/master/DEVELOPERS.md) page.

## Project status
TMBASIC is under development. Stay tuned!

## Motivation

There are many options for developing rich graphical user interfaces (GUIs) today, both on the web and on the desktop. Text user interfaces (TUIs) are an older technology, so why consider writing a TUI-based app using TMBASIC today?

**Text mode apps are easy to write.** Developing a high-quality GUI is time-consuming and requires significant expertise. TUIs are inherently more restricted and have fewer options for visual effects than GUIs, and as a result the skill ceiling is lower. A novice can produce TUIs that are just as good as those produced by experts.

**Text mode apps are easy to publish.** Deploying web apps on servers is a complicated matter, requiring experience in server operations. Similarly, distributing GUI toolkits like Qt or Electron requires shipping a lot of moving parts. TMBASIC apps are standalone executable files that can be simply copied to a user's computer. No additional files are required.

**Text mode apps are timeless.** As modern interface conventions continue to evolve, web and desktop apps look outdated after just a few years. A text user interface will not look any more dated in the future than it does today.

**Text mode apps do not need per-platform customizations.** Windows, macOS, and Linux each have their own user interface styles and conventions. A high-quality GUI must follow these conventions or risk looking out of place. The same is not true of TUIs, where expectations are the same on all platforms.

**TMBASIC makes it easy to get started.** Everything you need to develop and distribute sophisticated apps is included in TMBASIC. No other software is required. The BASIC language will feel familiar to anyone with programming experience and is easy to learn for new programmers.

## License
TMBASIC is open source software, but apps written in TMBASIC do not have to be open source. The following components are included in the TMBASIC development environment and in builds of user programs.

- tmbasic — _[MIT license](LICENSE)_
- [boost](https://www.boost.org/) — _[Boost Software License v1.0](https://github.com/electroly/tmbasic/blob/master/ext/boost/LICENSE_1_0.txt)_
- [immer](https://github.com/arximboldi/immer) — _[Boost Software License v1.0](https://github.com/electroly/tmbasic/blob/master/ext/immer/LICENSE)_
- [libstdc++](https://gcc.gnu.org/onlinedocs/libstdc++/) — _[GNU General Public License v3](https://github.com/electroly/tmbasic/blob/master/ext/gcc/GPL-3) with [GCC Runtime Library Exception v3.1](https://github.com/electroly/tmbasic/blob/master/ext/gcc/copyright)_
- [mpdecimal](https://www.bytereef.org/mpdecimal/) — _[BSD license](https://github.com/electroly/tmbasic/blob/master/ext/mpdecimal/LICENSE.txt)_
- [musl](https://musl.libc.org/) — _[MIT license](https://github.com/electroly/tmbasic/blob/master/ext/musl/COPYRIGHT)_
- [nameof](https://github.com/Neargye/nameof) — _[MIT license](https://github.com/electroly/tmbasic/blob/master/ext/nameof/LICENSE.txt)_
- [ncurses](https://en.wikipedia.org/wiki/Ncurses) — _[Ncurses license](https://github.com/electroly/tmbasic/blob/master/ext/ncurses/COPYING)_
- [tvision](https://github.com/magiblot/tvision) — _[Borland license](https://github.com/electroly/tmbasic/blob/master/ext/tvision/COPYRIGHT) (original code), [MIT license](https://github.com/electroly/tmbasic/blob/master/ext/tvision/COPYRIGHT) (additions)_

Pay close attention: executables built by TMBASIC are subject to the licenses above. You must obey these licenses if you distribute your compiled executables. These are permissive open source licenses that do not require you to release your BASIC source code. TMBASIC includes a `LICENSE.txt` file that combines these licenses for your convenience. Include this license file along with your executable when you distribute your TMBASIC apps.
