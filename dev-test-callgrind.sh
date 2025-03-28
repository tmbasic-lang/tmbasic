#!/bin/bash
set -e
OPTFLAGS="-O3 -g" EXTRADEFS="-DNDEBUG" make bin/test$(EXE_EXTENSION)
rm -f /code/callgrind.out /code/callgrind.txt
cd bin && valgrind --tool=callgrind --dump-instr=yes --trace-jump=yes --callgrind-out-file=/code/callgrind.out ./test
callgrind_annotate --include=/code/src --auto=yes /code/callgrind.out > /code/callgrind.txt
