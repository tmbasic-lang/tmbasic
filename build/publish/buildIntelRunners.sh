set -euxo pipefail
export PUBLISHDIR=/tmp/tmbasic-publish

cd $PUBLISHDIR/tmbasic/build

export DOCKER_FLAGS="--entrypoint /bin/bash"
export TTY_FLAG=" "

./win-x64.sh -ic "make clean && OPTFLAGS='-Os -flto' EXTRADEFS='-DNDEBUG' make runners"
cp -f ../bin/runners/102400.bz2     $PUBLISHDIR/win_x64_102400.bz2
cp -f ../bin/runners/524288.bsdiff  $PUBLISHDIR/win_x64_524288.bsdiff
cp -f ../bin/runners/1048576.bsdiff $PUBLISHDIR/win_x64_1048576.bsdiff
cp -f ../bin/runners/5242880.bsdiff $PUBLISHDIR/win_x64_5242880.bsdiff

./win-x86.sh -ic "make clean && OPTFLAGS='-Os -flto' EXTRADEFS='-DNDEBUG' make runners"
cp -f ../bin/runners/102400.bz2     $PUBLISHDIR/win_x86_102400.bz2
cp -f ../bin/runners/524288.bsdiff  $PUBLISHDIR/win_x86_524288.bsdiff
cp -f ../bin/runners/1048576.bsdiff $PUBLISHDIR/win_x86_1048576.bsdiff
cp -f ../bin/runners/5242880.bsdiff $PUBLISHDIR/win_x86_5242880.bsdiff

./linux-x64.sh -ic "make clean && OPTFLAGS='-Os -flto' EXTRADEFS='-DNDEBUG' make runners"
cp -f ../bin/runners/102400.bz2     $PUBLISHDIR/linux_x64_102400.bz2
cp -f ../bin/runners/524288.bsdiff  $PUBLISHDIR/linux_x64_524288.bsdiff
cp -f ../bin/runners/1048576.bsdiff $PUBLISHDIR/linux_x64_1048576.bsdiff
cp -f ../bin/runners/5242880.bsdiff $PUBLISHDIR/linux_x64_5242880.bsdiff

./linux-x86.sh -ic "make clean && OPTFLAGS='-Os -flto' EXTRADEFS='-DNDEBUG' make runners"
cp -f ../bin/runners/102400.bz2     $PUBLISHDIR/linux_x86_102400.bz2
cp -f ../bin/runners/524288.bsdiff  $PUBLISHDIR/linux_x86_524288.bsdiff
cp -f ../bin/runners/1048576.bsdiff $PUBLISHDIR/linux_x86_1048576.bsdiff
cp -f ../bin/runners/5242880.bsdiff $PUBLISHDIR/linux_x86_5242880.bsdiff

tar cf $PUBLISHDIR/runners_intel.tar $PUBLISHDIR/*.bz2 $PUBLISHDIR/*.bsdiff
