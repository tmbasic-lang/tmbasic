set -euxo pipefail
export PUBLISHDIR=/tmp/tmbasic-publish

cd $PUBLISHDIR/tmbasic/build

export DOCKER_FLAGS="--entrypoint /bin/bash"
export TTY_FLAG=" "

export MAKE_CMD="source /etc/profile.d/tmbasic.sh && make clean && OPTFLAGS='-Os' EXTRADEFS='-DNDEBUG' make runner"

./win-x64.sh -c "$MAKE_CMD"
cp -f ../bin/runner.gz $PUBLISHDIR/win_x64.gz

./win-x86.sh -c "$MAKE_CMD"
cp -f ../bin/runner.gz $PUBLISHDIR/win_x86.gz

./linux-x64.sh -c "$MAKE_CMD"
cp -f ../bin/runner.gz $PUBLISHDIR/linux_x64.gz

./linux-x86.sh -c "$MAKE_CMD"
cp -f ../bin/runner.gz $PUBLISHDIR/linux_x86.gz

cd $PUBLISHDIR
tar cf runners_intel.tar *.gz
