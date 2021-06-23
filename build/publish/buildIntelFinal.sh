set -euxo pipefail
export PUBLISHDIR=/tmp/tmbasic-publish

cd $PUBLISHDIR/tmbasic/build

export DOCKER_FLAGS="--entrypoint /bin/bash"
export TTY_FLAG=" "

export CLEAN_CMD="source /etc/profile.d/tmbasic.sh && make clean && mkdir obj"
export BUILD_CMD="source /etc/profile.d/tmbasic.sh && make release"

./win-x64.sh -c "$CLEAN_CMD"
mkdir -p $PUBLISHDIR/tmbasic/obj/resources/runners
pushd $PUBLISHDIR/tmbasic/obj/resources/runners && tar xf $PUBLISHDIR/runners.tar && popd
./win-x64.sh -c "$BUILD_CMD"
pushd $PUBLISHDIR/tmbasic/bin && zip -1 $PUBLISHDIR/tmbasic-win-x64.zip tmbasic.exe && popd

./win-x86.sh -c "$CLEAN_CMD"
mkdir -p $PUBLISHDIR/tmbasic/obj/resources/runners
pushd $PUBLISHDIR/tmbasic/obj/resources/runners && tar xf $PUBLISHDIR/runners.tar && popd
./win-x86.sh -c "$BUILD_CMD"
pushd $PUBLISHDIR/tmbasic/bin && zip -1 $PUBLISHDIR/tmbasic-win-x86.zip tmbasic.exe && popd

./linux-x64.sh -c "$CLEAN_CMD"
mkdir -p $PUBLISHDIR/tmbasic/obj/resources/runners
pushd $PUBLISHDIR/tmbasic/obj/resources/runners && tar xf $PUBLISHDIR/runners.tar && popd
./linux-x64.sh -c "$BUILD_CMD"
pushd $PUBLISHDIR/tmbasic/bin && tar zcf $PUBLISHDIR/tmbasic-linux-x64.tar.gz tmbasic && popd

./linux-x86.sh -c "$CLEAN_CMD"
mkdir -p $PUBLISHDIR/tmbasic/obj/resources/runners
pushd $PUBLISHDIR/tmbasic/obj/resources/runners && tar xf $PUBLISHDIR/runners.tar && popd
./linux-x86.sh -c "$BUILD_CMD"
pushd $PUBLISHDIR/tmbasic/bin && tar zcf $PUBLISHDIR/tmbasic-linux-x86.tar.gz tmbasic && popd
