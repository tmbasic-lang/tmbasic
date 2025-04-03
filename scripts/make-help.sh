#!/bin/bash
set -euxo pipefail

# Change to the repository root.
cd "$( dirname "${BASH_SOURCE[0]}" )"
cd ..

# Build the document builder
mkdir -p help-temp
c++ -o help-temp/buildDoc src/buildDoc.cpp -Wall -Werror -std=c++17 -lstdc++

# Build documentation
(cd doc && ../help-temp/buildDoc)

# Create help files
mkdir -p cmake/bin
rm -f cmake/bin/help.dat src/tmbasic/helpfile.h
tvhc help-temp/help.txt cmake/bin/help.dat src/tmbasic/helpfile.h
rm -rf help-temp
