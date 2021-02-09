set -euxo pipefail
export TESTDIR=/tmp/tmbasic-test

cd $TESTDIR/tmbasic/build

export DOCKER_FLAGS="--entrypoint /bin/bash"
export TTY_FLAG=" "

export TEST_CMD="make clean && make test"

./linux-arm64.sh -ic "$TEST_CMD"
./linux-arm32.sh -ic "$TEST_CMD"
