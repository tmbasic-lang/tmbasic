#!/bin/bash
# Input variables: $TARGET_OS, $ARCH
set -e

# Change to the repository root.
cd "$( dirname "${BASH_SOURCE[0]}" )"
cd ..

# Check that the input variables are set.
if [ -z "$TARGET_OS" ]; then
    echo "TARGET_OS is not set."
    exit 1
fi

if [ -z "$ARCH" ]; then
    echo "ARCH is not set."
    exit 1
fi

if [ "$TARGET_OS" = "win" ]; then
    TEST_CMD="mkdir -p /tmp/tmbasic-wine && HOME=/tmp/tmbasic-wine WINEPATH=/usr/$ARCH-w64-mingw32/bin wine test.exe"
else
    TEST_CMD="./test"
fi

./dev-build.sh
(cd cmake/bin && $TEST_CMD --gtest_shuffle)
