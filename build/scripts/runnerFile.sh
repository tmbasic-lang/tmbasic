#!/bin/bash
# inputs: $TARGET_OS $SHORT_ARCH $RUNNER_FILE
set -euo pipefail
export NAME=$(basename $RUNNER_FILE)
export BIN_FILE=obj/resources/runners/$NAME

# uncomment these lines to include a runner for the native platform in tmbasic builds.
# this adds a lot of time to the build but allows the "publish" feature to be tested.
# if [ "$NAME" == "${TARGET_OS}_${SHORT_ARCH}.gz" ]; then
#     cp -f bin/runner.gz $BIN_FILE
# fi

touch $BIN_FILE
