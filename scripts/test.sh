#!/bin/bash
set -e

# Change to the repository root.
cd "$( dirname "${BASH_SOURCE[0]}" )"
cd ..

scripts/make-tmbasic.sh
cmake/bin/test --gtest_shuffle
