#!/bin/bash
# updates the COMPILER_TEST() lines in src/test/CompilerTest.cpp
# run from the root /code dir

grep -v "^COMPILER_TEST" src/test/CompilerTest.cpp > /tmp/CompilerTest.cpp

pushd src/test/programs
find -type f \
    | grep "\.bas$" \
    | sed "s/\.bas//g; s:./::; s:/: :g" \
    | awk '{ print "COMPILER_TEST(" $1 ", " $2 ")" }' \
    >> /tmp/CompilerTest.cpp
popd

mv -f /tmp/CompilerTest.cpp src/test/CompilerTest.cpp
