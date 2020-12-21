#!/bin/bash
set -euxo pipefail

# create a tarball of the local source directory
export LOCAL_TAR=/tmp/tmbasic-local.tar.gz
rm -f $LOCAL_TAR
pushd ../../
rm -rf bin obj dist
tar zcf $LOCAL_TAR *
popd

# copy and extract the tarball on the Linux machines
export REMOTE_TAR=/tmp/tmbasic-remote.tar.gz
ssh -i $ARM_KEY $ARM_USER@$ARM_HOST "rm -f $REMOTE_TAR"
scp -i $ARM_KEY $LOCAL_TAR $ARM_USER@$ARM_HOST:$REMOTE_TAR
ssh -i $ARM_KEY $ARM_USER@$ARM_HOST "rm -rf /tmp/tmbasic && mkdir -p /tmp/tmbasic && cd /tmp/tmbasic/ && tar zxf $REMOTE_TAR"
ssh -i $X64_KEY $X64_USER@$X64_HOST "rm -f $REMOTE_TAR"
scp -i $X64_KEY $LOCAL_TAR $X64_USER@$X64_HOST:$REMOTE_TAR
ssh -i $X64_KEY $X64_USER@$X64_HOST "rm -rf /tmp/tmbasic && mkdir -p /tmp/tmbasic && cd /tmp/tmbasic/ && tar zxf $REMOTE_TAR"

# build the runner executables
ssh -i $X64_KEY $ARM_USER@$ARM_HOST "bash -is" < buildArmRunners.sh
ssh -i $X64_KEY $X64_USER@$X64_HOST "bash -is" < buildIntelRunners.sh

# copy the runners from the Linux machines
pushd /tmp
rm -rf runners
mkdir -p runners
cd runners
scp -i $ARM_KEY $ARM_USER@$ARM_HOST:/tmp/runner_linux_arm32 runner_linux_arm32
scp -i $ARM_KEY $ARM_USER@$ARM_HOST:/tmp/runner_linux_arm64 runner_linux_arm64
scp -i $X64_KEY $X64_USER@$X64_HOST:/tmp/runner_linux_x64 runner_linux_x64
scp -i $X64_KEY $X64_USER@$X64_HOST:/tmp/runner_linux_x86 runner_linux_x86
scp -i $X64_KEY $X64_USER@$X64_HOST:/tmp/runner_win_x64 runner_win_x64
scp -i $X64_KEY $X64_USER@$X64_HOST:/tmp/runner_win_x86 runner_win_x86
popd
