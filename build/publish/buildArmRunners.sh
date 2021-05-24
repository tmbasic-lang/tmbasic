set -euxo pipefail
export PUBLISHDIR=/tmp/tmbasic-publish

cd $PUBLISHDIR/tmbasic/build

export DOCKER_FLAGS="--entrypoint /bin/bash"
export TTY_FLAG=" "

export MAKE_CMD="source /etc/profile.d/tmbasic.sh && make clean && OPTFLAGS='-Os' EXTRADEFS='-DNDEBUG' make runners"

./linux-arm64.sh -c "$MAKE_CMD"
cp -f ../bin/runners/524288.bz2     $PUBLISHDIR/linux_arm64_524288.bz2
cp -f ../bin/runners/5242880.bsdiff $PUBLISHDIR/linux_arm64_5242880.bsdiff

./linux-arm32.sh -c "$MAKE_CMD"
cp -f ../bin/runners/524288.bz2     $PUBLISHDIR/linux_arm32_524288.bz2
cp -f ../bin/runners/5242880.bsdiff $PUBLISHDIR/linux_arm32_5242880.bsdiff

tar cf $PUBLISHDIR/runners_arm.tar $PUBLISHDIR/*.bz2 $PUBLISHDIR/*.bsdiff
