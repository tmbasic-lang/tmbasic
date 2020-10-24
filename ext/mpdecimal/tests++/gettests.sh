#!/bin/sh

cd ../tests && ./gettests.sh || exit 1
cd ../tests++ && cp -a ../tests/testdata . || exit 1

