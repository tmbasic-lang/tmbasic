#!/bin/bash
set -e

# Change to the repository root.
cd "$( dirname "${BASH_SOURCE[0]}" )"
cd ..

scripts/make-license.sh
scripts/make-help.sh
scripts/make-tmbasic.sh "$@"
