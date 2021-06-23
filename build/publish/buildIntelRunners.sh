set -euxo pipefail
export PUBLISHDIR=/tmp/tmbasic-publish

cd $PUBLISHDIR/tmbasic/build

export DOCKER_FLAGS="--entrypoint /bin/bash"
export TTY_FLAG=" "

export MAKE_CMD="source /etc/profile.d/tmbasic.sh && make clean && OPTFLAGS='-Os' EXTRADEFS='-DNDEBUG' make runners"

./win-x64.sh -c "$MAKE_CMD"
cp -f ../bin/runners/524288.bz2     $PUBLISHDIR/win_x64_524288.bz2
cp -f ../bin/runners/5242880.bsdiff $PUBLISHDIR/win_x64_5242880.bsdiff

./win-x86.sh -c "$MAKE_CMD"
cp -f ../bin/runners/524288.bz2     $PUBLISHDIR/win_x86_524288.bz2
cp -f ../bin/runners/5242880.bsdiff $PUBLISHDIR/win_x86_5242880.bsdiff

./linux-x64.sh -c "$MAKE_CMD"
cp -f ../bin/runners/524288.bz2     $PUBLISHDIR/linux_x64_524288.bz2
cp -f ../bin/runners/5242880.bsdiff $PUBLISHDIR/linux_x64_5242880.bsdiff

./linux-x86.sh -c "$MAKE_CMD"
cp -f ../bin/runners/524288.bz2     $PUBLISHDIR/linux_x86_524288.bz2
cp -f ../bin/runners/5242880.bsdiff $PUBLISHDIR/linux_x86_5242880.bsdiff

cd $PUBLISHDIR
tar cf runners_intel.tar *.bz2 *.bsdiff
