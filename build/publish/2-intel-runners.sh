set -euxo pipefail

rm -rf /tmp/tmbasic
#git clone https://github.com/electroly/tmbasic.git /tmp/tmbasic
#cd /tmp/tmbasic/build
cd /home/ubuntu/projects/tmbasic/build

export DOCKER_FLAGS="--entrypoint /bin/bash"
export TTY_FLAG=" "

./linux-x64.sh -c "make clean && make bin/runner"
mv ../bin/runner /tmp/runner_linux_x64

./linux-x86.sh -c "make clean && make bin/runner"
mv ../bin/runner /tmp/runner_linux_x86

./win-x64.sh -c "make clean && make bin/runner"
mv ../bin/runner.exe /tmp/runner_win_x64

./win-x86.sh -c "make clean && make bin/runner"
mv ../bin/runner.exe /tmp/runner_win_x86
