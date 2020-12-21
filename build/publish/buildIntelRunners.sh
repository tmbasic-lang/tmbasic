set -euxo pipefail

rm -rf /tmp/runner_*
cd /tmp/tmbasic/build

export DOCKER_FLAGS="--entrypoint /bin/bash"
export TTY_FLAG=" "

./win-x64.sh -ic "make clean && make bin/runner.exe"
mv ../bin/runner.exe /tmp/runner_win_x64

./win-x86.sh -ic "make clean && make bin/runner.exe"
mv ../bin/runner.exe /tmp/runner_win_x86

./linux-x64.sh -ic "make clean && make bin/runner"
mv ../bin/runner /tmp/runner_linux_x64

./linux-x86.sh -ic "make clean && make bin/runner"
mv ../bin/runner /tmp/runner_linux_x86
