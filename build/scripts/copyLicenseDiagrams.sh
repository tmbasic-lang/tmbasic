#!/bin/bash
set -euo pipefail
cp -f LICENSE obj/doc-temp/diagrams-license/license_tmbasic.txt
cp -f ext/boost/LICENSE_1_0.txt obj/doc-temp/diagrams-license/license_boost.txt
cp -f ext/musl/COPYRIGHT obj/doc-temp/diagrams-license/license_musl.txt
cp -f ext/immer/LICENSE obj/doc-temp/diagrams-license/license_immer.txt
cp -f ext/gcc/GPL-3 obj/doc-temp/diagrams-license/license_libstdc++_gpl3.txt
cp -f ext/gcc/copyright1 obj/doc-temp/diagrams-license/license_libstdc++_gcc1.txt
cp -f ext/gcc/copyright2 obj/doc-temp/diagrams-license/license_libstdc++_gcc2.txt
cp -f ext/mpdecimal/LICENSE.txt obj/doc-temp/diagrams-license/license_mpdecimal.txt
cp -f ext/nameof/LICENSE.txt obj/doc-temp/diagrams-license/license_nameof.txt
cp -f ext/ncurses/COPYING obj/doc-temp/diagrams-license/license_ncurses.txt
cp -f ext/tvision/COPYRIGHT obj/doc-temp/diagrams-license/license_tvision.txt
cp -f ext/bsdiff/LICENSE obj/doc-temp/diagrams-license/license_bsdiff.txt
cp -f ext/bzip2/COPYING obj/doc-temp/diagrams-license/license_bzip2.txt
