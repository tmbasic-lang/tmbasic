#!/bin/bash
# inputs: $TARGET_OS $SHORT_ARCH $RUNNER_FILE
set -euo pipefail
export NAME=$(basename $RUNNER_FILE)
export BIN_FILE=obj/resources/runners/$NAME

if [ "$NAME" == "${TARGET_OS}_${SHORT_ARCH}.gz" ]; then
    cp -f bin/runner.gz $BIN_FILE
fi

touch $BIN_FILE
