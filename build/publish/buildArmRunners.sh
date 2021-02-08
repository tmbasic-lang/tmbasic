set -euxo pipefail
export PUBLISHDIR=/tmp/tmbasic-publish

cd $PUBLISHDIR/tmbasic/build

export DOCKER_FLAGS="--entrypoint /bin/bash"
export TTY_FLAG=" "

./linux-arm64.sh -ic "make clean && OPTFLAGS='-Os -flto' EXTRADEFS='-DNDEBUG' make runners"
cp -f ../bin/runners/102400.bz2     $PUBLISHDIR/linux_arm64_102400.bz2
cp -f ../bin/runners/524288.bsdiff  $PUBLISHDIR/linux_arm64_524288.bsdiff
cp -f ../bin/runners/1048576.bsdiff $PUBLISHDIR/linux_arm64_1048576.bsdiff
cp -f ../bin/runners/5242880.bsdiff $PUBLISHDIR/linux_arm64_5242880.bsdiff

./linux-arm32.sh -ic "make clean && OPTFLAGS='-Os -flto' EXTRADEFS='-DNDEBUG' make runners"
cp -f ../bin/runners/102400.bz2     $PUBLISHDIR/linux_arm32_102400.bz2
cp -f ../bin/runners/524288.bsdiff  $PUBLISHDIR/linux_arm32_524288.bsdiff
cp -f ../bin/runners/1048576.bsdiff $PUBLISHDIR/linux_arm32_1048576.bsdiff
cp -f ../bin/runners/5242880.bsdiff $PUBLISHDIR/linux_arm32_5242880.bsdiff

tar cf $PUBLISHDIR/runners_arm.tar $PUBLISHDIR/*.bz2 $PUBLISHDIR/*.bsdiff
