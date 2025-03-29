#!/bin/bash
set -euxo pipefail

LICENSE_DIAGRAM_TXT_FILES=(
	obj/doc-temp/diagrams-license/license_tmbasic.txt
	obj/doc-temp/diagrams-license/license_boost.txt
	obj/doc-temp/diagrams-license/license_musl.txt
	obj/doc-temp/diagrams-license/license_immer.txt
	obj/doc-temp/diagrams-license/license_libstdc++_gpl3.txt
	obj/doc-temp/diagrams-license/license_libstdc++_gcc1.txt
	obj/doc-temp/diagrams-license/license_libstdc++_gcc2.txt
	obj/doc-temp/diagrams-license/license_mpdecimal.txt
	obj/doc-temp/diagrams-license/license_nameof.txt
	obj/doc-temp/diagrams-license/license_ncurses.txt
	obj/doc-temp/diagrams-license/license_tvision.txt
	obj/doc-temp/diagrams-license/license_fmt.txt
	obj/doc-temp/diagrams-license/license_scintilla.txt
	obj/doc-temp/diagrams-license/license_turbo.txt
	obj/doc-temp/diagrams-license/license_libzip.txt
	obj/doc-temp/diagrams-license/license_microtar.txt
	obj/doc-temp/diagrams-license/license_zlib.txt
	obj/doc-temp/diagrams-license/license_cli11.txt
	obj/doc-temp/diagrams-license/license_abseil.txt
	obj/doc-temp/diagrams-license/license_utf8proc.txt
)

# Create license diagram files
build/scripts/copyLicenses.sh

# Build the document builder
mkdir -p obj
c++ -o obj/buildDoc src/buildDoc.cpp -Wall -Werror -std=c++17 -lstdc++

# Build documentation
mkdir -p obj/resources/help
(cd doc && ../obj/buildDoc)

# Create help files
mkdir -p bin
rm -f bin/help.dat
rm -f src/tmbasic/helpfile.h
tvhc obj/resources/help/help.txt bin/help.dat src/tmbasic/helpfile.h
