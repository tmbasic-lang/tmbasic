#!/bin/bash
# Input variables: $TARGET_OS, $ARCH
set -e

# Check that the input variables are set.
if [ -z "$TARGET_OS" ]; then
    echo "TARGET_OS is not set."
    exit 1
fi

if [ -z "$ARCH" ]; then
    echo "ARCH is not set."
    exit 1
fi

# TEST_CMD: We run our unit test executable in "make test". For the Windows target, we use Wine since we cross-compile
# from Linux. This command is executed from the "bin" directory.
if [ "$TARGET_OS" = "win" ]; then
    TEST_CMD="mkdir -p /tmp/tmbasic-wine && HOME=/tmp/tmbasic-wine WINEPATH=/usr/$ARCH-w64-mingw32/bin wine test.exe"
else
    TEST_CMD="./test"
fi

make
(cd bin && $TEST_CMD --gtest_shuffle)
