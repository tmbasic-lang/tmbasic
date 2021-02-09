#!/bin/bash
set -euxo pipefail
export TESTDIR=/tmp/tmbasic-test

# we will use $TESTDIR on all three machines as a working area
rm -rf $TESTDIR
mkdir -p $TESTDIR
ssh -i $ARM_KEY $ARM_USER@$ARM_HOST "rm -rf $TESTDIR && mkdir -p $TESTDIR"
ssh -i $X64_KEY $X64_USER@$X64_HOST "rm -rf $TESTDIR && mkdir -p $TESTDIR"

# create a tarball of the local source directory
export LOCAL_TAR=$TESTDIR/tmbasic-local.tar.gz
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
export REMOTE_TAR=$TESTDIR/tmbasic-remote.tar.gz
ssh -i $ARM_KEY $ARM_USER@$ARM_HOST "rm -f $REMOTE_TAR"
scp -i $ARM_KEY $LOCAL_TAR $ARM_USER@$ARM_HOST:$REMOTE_TAR
ssh -i $ARM_KEY $ARM_USER@$ARM_HOST "rm -rf $TESTDIR/tmbasic && mkdir -p $TESTDIR/tmbasic && cd $TESTDIR/tmbasic/ && tar zxf $REMOTE_TAR"
ssh -i $X64_KEY $X64_USER@$X64_HOST "rm -f $REMOTE_TAR"
scp -i $X64_KEY $LOCAL_TAR $X64_USER@$X64_HOST:$REMOTE_TAR
ssh -i $X64_KEY $X64_USER@$X64_HOST "rm -rf $TESTDIR/tmbasic && mkdir -p $TESTDIR/tmbasic && cd $TESTDIR/tmbasic/ && tar zxf $REMOTE_TAR"

# run tests
pushd ../  # build directory
./mac-x64.sh -ic "make clean && make test"
popd

ssh -i $X64_KEY $ARM_USER@$ARM_HOST "bash -is" < runArmTests.sh
ssh -i $X64_KEY $X64_USER@$X64_HOST "bash -is" < runIntelTests.sh
