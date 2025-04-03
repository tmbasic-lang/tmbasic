#!/bin/bash
set -e

# Change to the repository root.
cd "$( dirname "${BASH_SOURCE[0]}" )"
cd ..

rm -rf cmake valgrind.txt
