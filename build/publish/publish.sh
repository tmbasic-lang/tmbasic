#!/bin/bash
set -euxo pipefail
export PUBLISHDIR=/tmp/tmbasic-publish

# we will use $PUBLISHDIR on both machines as a working area
rm -rf $PUBLISHDIR
mkdir -p $PUBLISHDIR/runners/
ssh -i $BUILD_KEY $BUILD_USER@$BUILD_HOST "rm -rf $PUBLISHDIR && mkdir -p $PUBLISHDIR"

# output will go into the dist folder
rm -rf ../../dist

#
# Mac runner
#

pushd ../  # build directory
./mac-arm64.sh -ic "make clean && make runner"
cp -f ../bin/runner.gz $PUBLISHDIR/runners/mac_arm64.gz

./mac-x64.sh -ic "make clean && make runner"
cp -f ../bin/runner.gz $PUBLISHDIR/runners/mac_x64.gz
popd

#
# Linux and Windows runners
#

# create a tarball of the local source directory
export LOCAL_TAR=$PUBLISHDIR/tmbasic-local.tar.gz
pushd ../../  # root of repository
tar zcf $LOCAL_TAR LICENSE Makefile build doc src
popd

# copy and extract the tarball on the Linux machines
export REMOTE_TAR=$PUBLISHDIR/tmbasic-remote.tar.gz
ssh -i $BUILD_KEY $BUILD_USER@$BUILD_HOST "rm -f $REMOTE_TAR"
scp -i $BUILD_KEY $LOCAL_TAR $BUILD_USER@$BUILD_HOST:$REMOTE_TAR
ssh -i $BUILD_KEY $BUILD_USER@$BUILD_HOST "rm -rf $PUBLISHDIR/tmbasic && mkdir -p $PUBLISHDIR/tmbasic && cd $PUBLISHDIR/tmbasic/ && tar zxf $REMOTE_TAR"

# build the runner executables
scp -i $BUILD_KEY buildRunners.sh $BUILD_USER@$BUILD_HOST:$PUBLISHDIR/buildRunners.sh
ssh -i $BUILD_KEY $BUILD_USER@$BUILD_HOST "bash $PUBLISHDIR/buildRunners.sh"

# copy the runners from the Linux machine
pushd $PUBLISHDIR/runners
scp -i $BUILD_KEY $BUILD_USER@$BUILD_HOST:$PUBLISHDIR/runners.tar runners.tar
tar xf runners.tar
rm -f runners.tar
popd

#
# Mac final build
#

mkdir -p ../../dist

pushd ../../  # root of repository
pushd build && ./mac-arm64.sh -c "make clean" && popd
mkdir -p obj/resources/runners/
cp -f $PUBLISHDIR/runners/*.gz obj/resources/runners/
pushd build && ./mac-arm64.sh -c "make" && popd
cd bin
zip -1 ../dist/tmbasic-mac-arm64.zip tmbasic
popd

pushd ../../  # root of repository
pushd build && ./mac-x64.sh -c "make clean" && popd
mkdir -p obj/resources/runners/
cp -f $PUBLISHDIR/runners/*.gz obj/resources/runners/
pushd build && ./mac-x64.sh -c "make" && popd
cd bin
zip -1 ../dist/tmbasic-mac-x64.zip tmbasic
popd

#
# Linux and Windows final builds
#

# copy the runners to the Linux machine
pushd $PUBLISHDIR/runners
tar cf $PUBLISHDIR/runners.tar *.gz
scp -i $BUILD_KEY $PUBLISHDIR/runners.tar $BUILD_USER@$BUILD_HOST:$PUBLISHDIR/runners.tar
popd

# build tmbasic
scp -i $BUILD_KEY buildFinal.sh $BUILD_USER@$BUILD_HOST:$PUBLISHDIR/buildFinal.sh
ssh -i $BUILD_KEY $BUILD_USER@$BUILD_HOST "bash $PUBLISHDIR/buildFinal.sh"

# copy the builds back from the Linux machine
pushd ../../  # root of repository
scp -i $BUILD_KEY $BUILD_USER@$BUILD_HOST:$PUBLISHDIR/tmbasic-linux-arm64.tar.gz dist/tmbasic-linux-arm64.tar.gz
scp -i $BUILD_KEY $BUILD_USER@$BUILD_HOST:$PUBLISHDIR/tmbasic-linux-arm32.tar.gz dist/tmbasic-linux-arm32.tar.gz
scp -i $BUILD_KEY $BUILD_USER@$BUILD_HOST:$PUBLISHDIR/tmbasic-linux-x64.tar.gz dist/tmbasic-linux-x64.tar.gz
scp -i $BUILD_KEY $BUILD_USER@$BUILD_HOST:$PUBLISHDIR/tmbasic-linux-x86.tar.gz dist/tmbasic-linux-x86.tar.gz
scp -i $BUILD_KEY $BUILD_USER@$BUILD_HOST:$PUBLISHDIR/tmbasic-win-x64.zip dist/tmbasic-win-x64.zip
scp -i $BUILD_KEY $BUILD_USER@$BUILD_HOST:$PUBLISHDIR/tmbasic-win-x86.zip dist/tmbasic-win-x86.zip
popd

#
# Cleanup
#

rm -rf $PUBLISHDIR
ssh -i $BUILD_KEY $BUILD_USER@$BUILD_HOST "rm -rf $PUBLISHDIR"

set +x
echo
echo
ls -l ../../dist
echo "Finished at: $(date)"
