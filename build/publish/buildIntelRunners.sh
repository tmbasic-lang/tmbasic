set -euxo pipefail
export PUBLISHDIR=/tmp/tmbasic-publish

rm -rf $PUBLISHDIR/runner_*
cd $PUBLISHDIR/tmbasic/build

export DOCKER_FLAGS="--entrypoint /bin/bash"
export TTY_FLAG=" "

./win-x64.sh -ic "make clean && OPTFLAGS='-Os -flto' EXTRADEFS='-DNDEBUG' make bin/runner.exe"
mv ../bin/runner.exe $PUBLISHDIR/runner_win_x64

./win-x86.sh -ic "make clean && OPTFLAGS='-Os -flto' EXTRADEFS='-DNDEBUG' make bin/runner.exe"
mv ../bin/runner.exe $PUBLISHDIR/runner_win_x86

./linux-x64.sh -ic "make clean && OPTFLAGS='-Os -flto' EXTRADEFS='-DNDEBUG' make bin/runner"
mv ../bin/runner $PUBLISHDIR/runner_linux_x64

./linux-x86.sh -ic "make clean && OPTFLAGS='-Os -flto' EXTRADEFS='-DNDEBUG' make bin/runner"
mv ../bin/runner $PUBLISHDIR/runner_linux_x86
