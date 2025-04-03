#!/bin/bash
set -e

# Change to the repository root.
cd "$( dirname "${BASH_SOURCE[0]}" )"
cd ..

scripts/make-tmbasic.sh
cpplint --quiet --recursive --repository=src --exclude=src/boost --exclude=src/tmbasic/helpfile.h src
