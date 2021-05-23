#!/bin/bash
set -euxo pipefail
export TESTDIR=/tmp/tmbasic-test

# we will use $TESTDIR on all three machines as a working area
rm -rf $TESTDIR
mkdir -p $TESTDIR
ssh -i $ARM_KEY $ARM_USER@$ARM_HOST "rm -rf $TESTDIR && mkdir -p $TESTDIR"
ssh -i $X64_KEY $X64_USER@$X64_HOST "rm -rf $TESTDIR && mkdir -p $TESTDIR"

# set up mac deps
pushd ../  # build directory
./mac-arm64.sh -c "make clean"
./mac-x64.sh -c "make clean"
popd

# create a tarball of the local source directory
export LOCAL_TAR=$TESTDIR/tmbasic-local.tar.gz
pushd ../../  # root of repository
mv mac-x64 .mac-x64
mv mac-arm64 .mac-arm64
tar zcf $LOCAL_TAR *  # wildcard excludes dot files
mv .mac-x64 mac-x64
mv .mac-arm64 mac-arm64
popd

# copy and extract the tarball on the Linux machines
export REMOTE_TAR=$TESTDIR/tmbasic-remote.tar.gz
ssh -i $ARM_KEY $ARM_USER@$ARM_HOST "rm -f $REMOTE_TAR"
scp -i $ARM_KEY $LOCAL_TAR $ARM_USER@$ARM_HOST:$REMOTE_TAR
ssh -i $ARM_KEY $ARM_USER@$ARM_HOST "rm -rf $TESTDIR/tmbasic && mkdir -p $TESTDIR/tmbasic && cd $TESTDIR/tmbasic/ && tar zxf $REMOTE_TAR"
ssh -i $X64_KEY $X64_USER@$X64_HOST "rm -f $REMOTE_TAR"
scp -i $X64_KEY $LOCAL_TAR $X64_USER@$X64_HOST:$REMOTE_TAR
ssh -i $X64_KEY $X64_USER@$X64_HOST "rm -rf $TESTDIR/tmbasic && mkdir -p $TESTDIR/tmbasic && cd $TESTDIR/tmbasic/ && tar zxf $REMOTE_TAR"

# run tests
pushd ../  # build directory
./mac-arm64.sh -c "make clean && make test"
./mac-x64.sh -c "make clean && make test"
popd

scp -i $ARM_KEY runArmTests.sh $ARM_USER@$ARM_HOST:$TESTDIR/runArmTests.sh
ssh -i $ARM_KEY $ARM_USER@$ARM_HOST "bash $TESTDIR/runArmTests.sh"

scp -i $X64_KEY runIntelTests.sh $X64_USER@$X64_HOST:$TESTDIR/runIntelTests.sh
ssh -i $X64_KEY $X64_USER@$X64_HOST "bash $TESTDIR/runIntelTests.sh"
