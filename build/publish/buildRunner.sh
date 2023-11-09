# Inputs: $PLATFORM $OUTPUT_GZ_PATH
set -euxo pipefail

export DOCKER_FLAGS="--entrypoint /bin/bash"
export TTY_FLAG=" "
export MAKE_CMD="source /etc/profile.d/tmbasic.sh && make clean && OPTFLAGS='-Os' EXTRADEFS='-DNDEBUG' make runner"

pushd build

./${PLATFORM}.sh -c "$MAKE_CMD"
cp -f ../bin/runner.gz "$OUTPUT_GZ_PATH"

popd
