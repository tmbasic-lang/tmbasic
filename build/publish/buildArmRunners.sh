set -euxo pipefail

rm -rf /tmp/runner_*
cd /tmp/tmbasic/build

export DOCKER_FLAGS="--entrypoint /bin/bash"
export TTY_FLAG=" "

./linux-arm64.sh -ic "make clean && make bin/runner"
mv ../bin/runner /tmp/runner_linux_arm64

./linux-arm32.sh -ic "make clean && make bin/runner"
mv ../bin/runner /tmp/runner_linux_arm32
