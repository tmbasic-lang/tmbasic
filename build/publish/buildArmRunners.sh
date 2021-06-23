set -euxo pipefail
export PUBLISHDIR=/tmp/tmbasic-publish
cd $PUBLISHDIR/tmbasic/build

export DOCKER_FLAGS="--entrypoint /bin/bash"
export TTY_FLAG=" "

export MAKE_CMD="source /etc/profile.d/tmbasic.sh && make clean && OPTFLAGS='-Os' EXTRADEFS='-DNDEBUG' make runner"

./linux-arm64.sh -c "$MAKE_CMD"
cp -f ../bin/runner.gz $PUBLISHDIR/linux_arm64.gz

./linux-arm32.sh -c "$MAKE_CMD"
cp -f ../bin/runner.gz $PUBLISHDIR/linux_arm32.gz

cd $PUBLISHDIR
tar cf runners_arm.tar *.gz
