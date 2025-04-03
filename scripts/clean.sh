#!/bin/bash
set -e

# Change to the repository root.
cd "$( dirname "${BASH_SOURCE[0]}" )"
cd ..

rm -rf cmake win-arm64 win-x64 win-x86 win-native mac-arm64 mac-x64 valgrind.txt
