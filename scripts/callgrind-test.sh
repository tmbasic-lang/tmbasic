#!/bin/bash
set -e

# Change to the repository root.
cd "$( dirname "${BASH_SOURCE[0]}" )"
cd ..

scripts/make-tmbasic.sh OPTFLAGS="-O3 -g" EXTRADEFS="-DNDEBUG"
rm -f /code/callgrind.out /code/callgrind.txt
cd bin && valgrind --tool=callgrind --dump-instr=yes --trace-jump=yes --callgrind-out-file=/code/callgrind.out ./test
callgrind_annotate --include=/code/src --auto=yes /code/callgrind.out > /code/callgrind.txt
