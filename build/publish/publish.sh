#!/bin/bash
set -euxo pipefail
export PUBLISHDIR=/tmp/tmbasic-publish

# we will use $PUBLISHDIR on all three machines as a working area
rm -rf $PUBLISHDIR
mkdir -p $PUBLISHDIR/runners/
ssh -i $ARM_KEY $ARM_USER@$ARM_HOST "rm -rf $PUBLISHDIR && mkdir -p $PUBLISHDIR"
ssh -i $X64_KEY $X64_USER@$X64_HOST "rm -rf $PUBLISHDIR && mkdir -p $PUBLISHDIR"

# output will go into the dist folder
rm -rf ../../dist

#
# Mac runner
#

pushd ../  # build directory
./mac-x64.sh -ic "make clean && make bin/runner"
cp -f ../bin/runner $PUBLISHDIR/runners/runner_mac_x64
popd

#
# Linux and Windows runners
#

# create a tarball of the local source directory
export LOCAL_TAR=$PUBLISHDIR/tmbasic-local.tar.gz
pushd ../../  # root of repository
mv bin .bin
mv obj .obj
mv mac .mac
tar zcf $LOCAL_TAR *  # wildcard excludes dot files
mv .bin bin
mv .obj obj
mv .mac mac
popd

# copy and extract the tarball on the Linux machines
export REMOTE_TAR=$PUBLISHDIR/tmbasic-remote.tar.gz
ssh -i $ARM_KEY $ARM_USER@$ARM_HOST "rm -f $REMOTE_TAR"
scp -i $ARM_KEY $LOCAL_TAR $ARM_USER@$ARM_HOST:$REMOTE_TAR
ssh -i $ARM_KEY $ARM_USER@$ARM_HOST "rm -rf $PUBLISHDIR/tmbasic && mkdir -p $PUBLISHDIR/tmbasic && cd $PUBLISHDIR/tmbasic/ && tar zxf $REMOTE_TAR"
ssh -i $X64_KEY $X64_USER@$X64_HOST "rm -f $REMOTE_TAR"
scp -i $X64_KEY $LOCAL_TAR $X64_USER@$X64_HOST:$REMOTE_TAR
ssh -i $X64_KEY $X64_USER@$X64_HOST "rm -rf $PUBLISHDIR/tmbasic && mkdir -p $PUBLISHDIR/tmbasic && cd $PUBLISHDIR/tmbasic/ && tar zxf $REMOTE_TAR"

# build the runner executables
ssh -i $X64_KEY $ARM_USER@$ARM_HOST "bash -is" < buildArmRunners.sh
ssh -i $X64_KEY $X64_USER@$X64_HOST "bash -is" < buildIntelRunners.sh

# copy the runners from the Linux machines
pushd $PUBLISHDIR/runners
scp -i $ARM_KEY $ARM_USER@$ARM_HOST:$PUBLISHDIR/runner_linux_arm32 runner_linux_arm32
scp -i $ARM_KEY $ARM_USER@$ARM_HOST:$PUBLISHDIR/runner_linux_arm64 runner_linux_arm64
scp -i $X64_KEY $X64_USER@$X64_HOST:$PUBLISHDIR/runner_linux_x64 runner_linux_x64
scp -i $X64_KEY $X64_USER@$X64_HOST:$PUBLISHDIR/runner_linux_x86 runner_linux_x86
scp -i $X64_KEY $X64_USER@$X64_HOST:$PUBLISHDIR/runner_win_x64 runner_win_x64
scp -i $X64_KEY $X64_USER@$X64_HOST:$PUBLISHDIR/runner_win_x86 runner_win_x86
popd

#
# Mac final build
#

mkdir -p ../../dist

pushd ../../  # root of repository
cp -f $PUBLISHDIR/runners/runner* obj/
cd build
./mac-x64.sh -ic "make && make test"
cd ..
zip -9 dist/tmbasic-mac-x64.zip bin/tmbasic bin/LICENSE.txt bin/LICENSE.tmbasic.txt
popd

#
# Linux and Windows final builds
#

# copy the runners to the Linux machines
pushd $PUBLISHDIR/runners
tar zcf $PUBLISHDIR/runners.tar.gz runner_*
scp -i $ARM_KEY $PUBLISHDIR/runners.tar.gz $ARM_USER@$ARM_HOST:$PUBLISHDIR/runners.tar.gz
scp -i $X64_KEY $PUBLISHDIR/runners.tar.gz $X64_USER@$X64_HOST:$PUBLISHDIR/runners.tar.gz
popd

# build tmbasic
ssh -i $X64_KEY $ARM_USER@$ARM_HOST "bash -is" < buildArmFinal.sh
ssh -i $X64_KEY $X64_USER@$X64_HOST "bash -is" < buildIntelFinal.sh

# copy the builds back from the Linux machines
pushd ../../  # root of repository
scp -i $ARM_KEY $ARM_USER@$ARM_HOST:$PUBLISHDIR/tmbasic-linux-arm32.tar.gz dist/tmbasic-linux-arm32.tar.gz
scp -i $ARM_KEY $ARM_USER@$ARM_HOST:$PUBLISHDIR/tmbasic-linux-arm64.tar.gz dist/tmbasic-linux-arm64.tar.gz
scp -i $X64_KEY $X64_USER@$X64_HOST:$PUBLISHDIR/tmbasic-linux-x64.tar.gz dist/tmbasic-linux-x64.tar.gz
scp -i $X64_KEY $X64_USER@$X64_HOST:$PUBLISHDIR/tmbasic-linux-x86.tar.gz dist/tmbasic-linux-x86.tar.gz
scp -i $X64_KEY $X64_USER@$X64_HOST:$PUBLISHDIR/tmbasic-win-x64.zip dist/tmbasic-win-x64.zip
scp -i $X64_KEY $X64_USER@$X64_HOST:$PUBLISHDIR/tmbasic-win-x86.zip dist/tmbasic-win-x86.zip
popd

#
# Cleanup
#

rm -rf $PUBLISHDIR
ssh -i $ARM_KEY $ARM_USER@$ARM_HOST "rm -rf $PUBLISHDIR"
ssh -i $X64_KEY $X64_USER@$X64_HOST "rm -rf $PUBLISHDIR"

set +x
ls -l ../../dist
echo "Finished at: $(date)"
