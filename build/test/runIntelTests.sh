set -euxo pipefail
export TESTDIR=/tmp/tmbasic-test

cd $TESTDIR/tmbasic/build

export DOCKER_FLAGS="--entrypoint /bin/bash"
export TTY_FLAG=" "

export TEST_CMD="make clean && make test"

./win-x64.sh -ic "source /etc/profile.d/tmbasic.sh && $TEST_CMD"
./win-x86.sh -ic "source /etc/profile.d/tmbasic.sh && $TEST_CMD"
./linux-x64.sh -ic "source /etc/profile.d/tmbasic.sh && $TEST_CMD"
./linux-x86.sh -ic "source /etc/profile.d/tmbasic.sh && $TEST_CMD"
