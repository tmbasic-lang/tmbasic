#!/bin/bash
set -e

# Change to the repository root.
cd "$( dirname "${BASH_SOURCE[0]}" )"
cd ..

# Valgrind is incompatible with ASan; make sure to build with DISABLE_SANITIZERS=1.
scripts/make-tmbasic.sh -DDISABLE_SANITIZERS=1
valgrind --log-file=valgrind.txt cmake/bin/tmbasic || cat valgrind.txt
