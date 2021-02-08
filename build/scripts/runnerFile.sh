#!/bin/bash
# inputs: $TARGET_OS $SHORT_ARCH $RUNNER_FILE
set -euo pipefail
export NAME=$(basename $RUNNER_FILE)
export BIN_FILE=obj/resources/runners/$NAME

if [ ! -f $BIN_FILE ]; then
    touch $BIN_FILE
fi

if [ "$NAME" == "${TARGET_OS}_${SHORT_ARCH}_102400.bz2" ]; then
    cp -f bin/runners/102400.bz2 $BIN_FILE
fi
if [ "$NAME" == "${TARGET_OS}_${SHORT_ARCH}_524288.bsdiff" ]; then
    cp -f bin/runners/524288.bsdiff $BIN_FILE
fi
if [ "$NAME" == "${TARGET_OS}_${SHORT_ARCH}_1048576.bsdiff" ]; then
    cp -f bin/runners/1048576.bsdiff $BIN_FILE
fi
if [ "$NAME" == "${TARGET_OS}_${SHORT_ARCH}_5242880.bsdiff" ]; then
    cp -f bin/runners/5242880.bsdiff $BIN_FILE
fi
