#!/bin/bash
set -eo pipefail

# Change to the repository root.
cd "$( dirname "${BASH_SOURCE[0]}" )"
cd ..

scripts/make-tmbasic.sh
find src/ -type f \( -iname \*.h -o -iname \*.cpp \) -not -path "src/boost/*" -not -path "src/tmbasic/helpfile.h" | xargs clang-format -i
