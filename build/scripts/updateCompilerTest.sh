#!/bin/bash
# updates the COMPILER_TEST() lines in src/test/CompilerTest.cpp
# run from the root /code dir

grep -v "^COMPILER_TEST" src/test/CompilerTest.cpp > /tmp/CompilerTest.cpp

ls src/test/programs \
    | grep "\.bas$" \
    | xargs -n 1 basename \
    | sed "s/\.bas//g" \
    | awk '{ print "COMPILER_TEST(" $1 ")" }' \
    >> /tmp/CompilerTest.cpp

mv -f /tmp/CompilerTest.cpp src/test/CompilerTest.cpp
