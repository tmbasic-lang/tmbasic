set -euxo pipefail
export PUBLISHDIR=/tmp/tmbasic-publish

rm -rf $PUBLISHDIR/runner_*
cd $PUBLISHDIR/tmbasic/build

export DOCKER_FLAGS="--entrypoint /bin/bash"
export TTY_FLAG=" "

./linux-arm64.sh -ic "make clean && OPTFLAGS='-Os -flto' EXTRADEFS='-DNDEBUG' make bin/runner"
mv ../bin/runner $PUBLISHDIR/runner_linux_arm64

./linux-arm32.sh -ic "make clean && OPTFLAGS='-Os -flto' EXTRADEFS='-DNDEBUG' make bin/runner"
mv ../bin/runner $PUBLISHDIR/runner_linux_arm32
