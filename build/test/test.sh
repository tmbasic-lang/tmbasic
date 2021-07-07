#!/bin/bash
set -euxo pipefail
export TESTDIR=/tmp/tmbasic-test

# we will use $TESTDIR on all three machines as a working area
rm -rf $TESTDIR && mkdir -p $TESTDIR
ssh -i $BUILD_KEY $BUILD_USER@$BUILD_HOST "rm -rf $TESTDIR && mkdir -p $TESTDIR"
ssh -i $ARM_KEY $ARM_USER@$ARM_HOST "rm -rf $TESTDIR && mkdir -p $TESTDIR"
ssh -i $X64_KEY $X64_USER@$X64_HOST "rm -rf $TESTDIR && mkdir -p $TESTDIR"

cd ../  # build directory

# set up mac environment. this will also download deps before we copy the working dir to the Linux machine.
./mac-arm64.sh -c "make clean"
./mac-x64.sh -c "make clean"

# copy the tmbasic directory to the Linux machines
function copyAndExtractTmbasicDir {
    local REMOTE_KEY=$1
    local REMOTE_USER=$2
    local REMOTE_HOST=$3
    local FILES=$4
    local REMOTE_TAR=$TESTDIR/tmbasic-remote.tar.gz

    # create a tarball of the local source directory
    local LOCAL_TAR=$TESTDIR/tmbasic-local.tar.gz
    pushd ../  # root of repository
    rm -f build/files/sysroot-* build/files/deps.tar $LOCAL_TAR
    tar zcf $LOCAL_TAR $4
    popd

    # copy and extract
    ssh -i $REMOTE_KEY $REMOTE_USER@$REMOTE_HOST "rm -f $REMOTE_TAR"
    scp -i $REMOTE_KEY $LOCAL_TAR $REMOTE_USER@$REMOTE_HOST:$REMOTE_TAR
    ssh -i $REMOTE_KEY $REMOTE_USER@$REMOTE_HOST "rm -rf $TESTDIR/tmbasic && mkdir -p $TESTDIR/tmbasic && cd $TESTDIR/tmbasic/ && tar zxf $REMOTE_TAR"
}

copyAndExtractTmbasicDir $BUILD_KEY $BUILD_USER $BUILD_HOST "build doc src Makefile LICENSE"
copyAndExtractTmbasicDir $X64_KEY $X64_USER $X64_HOST "src"
copyAndExtractTmbasicDir $ARM_KEY $ARM_USER $ARM_HOST "src"

# build Dockerfile.wine on x64 machine
ssh -i $X64_KEY $X64_USER@$X64_HOST "rm -rf $TESTDIR/wine && mkdir -p $TESTDIR/wine"
scp -i $X64_KEY test/Dockerfile.wine $X64_USER@$X64_HOST:$TESTDIR/wine/Dockerfile.wine
ssh -i $X64_KEY $X64_USER@$X64_HOST "cd $TESTDIR/wine/ ; \
    [ \"\$(docker image ls tmbasic-wine | wc -l)\" == \"1\" ] && docker build -t tmbasic-wine -f Dockerfile.wine ."

# windows tests
function runWindowsTest {
    local ARCH=$1

    # build test.exe
    ssh -i $BUILD_KEY $BUILD_USER@$BUILD_HOST "cd $TESTDIR/tmbasic/build && \
        export DOCKER_FLAGS=\"--entrypoint /bin/bash\" && export TTY_FLAG=\" \" && \
        ./win-$ARCH.sh -ic \"source /etc/profile.d/tmbasic.sh && make clean && make bin/test.exe && gzip -1 bin/test.exe\""
    scp -i $BUILD_KEY $BUILD_USER@$BUILD_HOST:$TESTDIR/tmbasic/bin/test.exe.gz $TESTDIR/test.exe.gz
    
    # copy to x64 machine and run
    ssh -i $X64_KEY $X64_USER@$X64_HOST "mkdir -p $TESTDIR/tmbasic/bin && rm -f $TESTDIR/tmbasic/bin/test.exe $TESTDIR/tmbasic/bin/test.exe.gz"
    scp -i $X64_KEY $TESTDIR/test.exe.gz $X64_USER@$X64_HOST:$TESTDIR/tmbasic/bin/test.exe.gz
    ssh -i $X64_KEY $X64_USER@$X64_HOST "cd $TESTDIR/tmbasic/bin && gunzip test.exe.gz && cd .. && \
        docker run --rm --volume \"\$PWD:/code\" tmbasic-wine /bin/bash -ic \"cd /code/bin && WINEPATH=/usr/x86_64-w64-mingw32/bin wine64 test.exe\""
}

runWindowsTest "x64"
runWindowsTest "x86"

# linux tests
function runLinuxTest {
    local REMOTE_KEY=$1
    local REMOTE_USER=$2
    local REMOTE_HOST=$3
    local ARCH=$4

    # build test
    ssh -i $BUILD_KEY $BUILD_USER@$BUILD_HOST "cd $TESTDIR/tmbasic/build && \
        export DOCKER_FLAGS=\"--entrypoint /bin/bash\" && export TTY_FLAG=\" \" && \
        ./linux-$ARCH.sh -ic \"source /etc/profile.d/tmbasic.sh && make clean && make bin/test && gzip -1 bin/test\""
    rm -f $TESTDIR/test.gz
    scp -i $BUILD_KEY $BUILD_USER@$BUILD_HOST:$TESTDIR/tmbasic/bin/test.gz $TESTDIR/test.gz

    # copy to x64/arm64 machine and run
    ssh -i $REMOTE_KEY $REMOTE_USER@$REMOTE_HOST "mkdir -p $TESTDIR/tmbasic/bin && rm -f $TESTDIR/tmbasic/bin/test $TESTDIR/tmbasic/bin/test.gz"
    scp -i $REMOTE_KEY $TESTDIR/test.gz $REMOTE_USER@$REMOTE_HOST:$TESTDIR/tmbasic/bin/test.gz
    ssh -i $REMOTE_KEY $REMOTE_USER@$REMOTE_HOST "cd $TESTDIR/tmbasic/bin && gunzip test.gz && cd ../bin && ./test"
}

runLinuxTest $ARM_KEY $ARM_USER $ARM_HOST "arm64"
runLinuxTest $ARM_KEY $ARM_USER $ARM_HOST "arm32"
runLinuxTest $X64_KEY $X64_USER $X64_HOST "x64"
runLinuxTest $X64_KEY $X64_USER $X64_HOST "x86"

# mac tests
./mac-arm64.sh -c "make clean && make test"
./mac-x64.sh -c "make clean && make test"

# cleanup
ssh -i $BUILD_KEY $BUILD_USER@$BUILD_HOST "rm -rf $TESTDIR"
ssh -i $ARM_KEY $ARM_USER@$ARM_HOST "rm -rf $TESTDIR"
ssh -i $X64_KEY $X64_USER@$X64_HOST "rm -rf $TESTDIR"
