#!/bin/bash
set -e
# Valgrind is incompatible with ASan.
DISABLE_SANITIZERS=1 make
valgrind --log-file=valgrind.txt bin/tmbasic || cat valgrind.txt
