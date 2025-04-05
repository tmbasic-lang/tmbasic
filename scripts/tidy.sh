#!/bin/bash
set -euo pipefail

# Change to the repository root.
cd "$( dirname "${BASH_SOURCE[0]}" )"
cd ..

if [ $# -eq 0 ]; then
    # No arguments provided - tidy all files
    rm -rf cmake
    scripts/make-tmbasic.sh -DUSE_PCH=OFF
    echo > tidy.log

    find src/ -type f -name "*.cpp" -print0 \
        | xargs -0 -P $(nproc) -I {} scripts/tidy.sh "{}" 2>&1
else
    # Arguments provided - tidy specific files
    echo "$@"
    clang-tidy "$@" --quiet --fix --extra-arg=-Wno-unknown-warning-option -p cmake/compile_commands.json 2>&1 \
        | grep -v "warnings generated" \
        | tee -a tidy.log
fi
