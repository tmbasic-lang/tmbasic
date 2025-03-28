#!/usr/bin/bash
set -eo pipefail
find src/ -type f \( -iname \*.h -o -iname \*.cpp \) -not -path "src/boost/*" | xargs clang-format -i
