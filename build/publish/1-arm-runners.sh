set -euxo pipefail

rm -rf /tmp/tmbasic
#git clone https://github.com/electroly/tmbasic.git /tmp/tmbasic
#cd /tmp/tmbasic/build
cd /home/ubuntu/projects/tmbasic/build

export DOCKER_FLAGS="--entrypoint /bin/bash"
export TTY_FLAG=" "

./linux-arm64.sh -c "make clean && make bin/runner"
mv ../bin/runner /tmp/runner_linux_arm64

./linux-arm32.sh -c "make clean && make bin/runner"
mv ../bin/runner /tmp/runner_linux_arm32
