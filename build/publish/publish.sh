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
./mac-arm64.sh -ic "make clean && make runners"
cp -f ../bin/runners/524288.bz2     $PUBLISHDIR/runners/mac_arm64_524288.bz2
cp -f ../bin/runners/5242880.bsdiff $PUBLISHDIR/runners/mac_arm64_5242880.bsdiff

./mac-x64.sh -ic "make clean && make runners"
cp -f ../bin/runners/524288.bz2     $PUBLISHDIR/runners/mac_x64_524288.bz2
cp -f ../bin/runners/5242880.bsdiff $PUBLISHDIR/runners/mac_x64_5242880.bsdiff
popd

#
# Linux and Windows runners
#

# create a tarball of the local source directory
export LOCAL_TAR=$PUBLISHDIR/tmbasic-local.tar.gz
pushd ../../  # root of repository
mv bin .bin
mv obj .obj
mv mac-arm64 .mac-arm64
mv mac-x64 .mac-x64
tar zcf $LOCAL_TAR *  # wildcard excludes dot files
mv .bin bin
mv .obj obj
mv .mac-arm64 mac-arm64
mv .mac-x64 mac-x64
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
scp -i $ARM_KEY buildArmRunners.sh $ARM_USER@$ARM_HOST:$PUBLISHDIR/buildArmRunners.sh
ssh -i $ARM_KEY $ARM_USER@$ARM_HOST "bash $PUBLISHDIR/buildArmRunners.sh"

scp -i $X64_KEY buildIntelRunners.sh $X64_USER@$X64_HOST:$PUBLISHDIR/buildIntelRunners.sh
ssh -i $X64_KEY $X64_USER@$X64_HOST "bash $PUBLISHDIR/buildIntelRunners.sh"

# copy the runners from the Linux machines
pushd $PUBLISHDIR/runners
scp -i $ARM_KEY $ARM_USER@$ARM_HOST:$PUBLISHDIR/runners_arm.tar runners_arm.tar
tar xf runners_arm.tar
rm -f runners_arm.tar

scp -i $X64_KEY $X64_USER@$X64_HOST:$PUBLISHDIR/runners_intel.tar runners_intel.tar
tar xf runners_intel.tar
rm -f runners_intel.tar
popd

#
# Mac final build
#

mkdir -p ../../dist

pushd ../../  # root of repository
pushd build && ./mac-arm64.sh -c "make clean" && popd
mkdir -p obj/resources/runners/
cp -f $PUBLISHDIR/runners/*.bz2 $PUBLISHDIR/runners/*.bsdiff obj/resources/runners/
pushd build && ./mac-arm64.sh -c "make" && popd
cd bin
zip -9 ../dist/tmbasic-mac-arm64.zip tmbasic
popd

pushd ../../  # root of repository
pushd build && ./mac-x64.sh -c "make clean" && popd
mkdir -p obj/resources/runners/
cp -f $PUBLISHDIR/runners/*.bz2 $PUBLISHDIR/runners/*.bsdiff obj/resources/runners/
pushd build && ./mac-x64.sh -c "make" && popd
cd bin
zip -9 ../dist/tmbasic-mac-x64.zip tmbasic
popd

#
# Linux and Windows final builds
#

# copy the runners to the Linux machines
pushd $PUBLISHDIR/runners
tar cf $PUBLISHDIR/runners.tar *.bz2 *.bsdiff
scp -i $ARM_KEY $PUBLISHDIR/runners.tar $ARM_USER@$ARM_HOST:$PUBLISHDIR/runners.tar
scp -i $X64_KEY $PUBLISHDIR/runners.tar $X64_USER@$X64_HOST:$PUBLISHDIR/runners.tar
popd

# build tmbasic
scp -i $ARM_KEY buildArmFinal.sh $ARM_USER@$ARM_HOST:$PUBLISHDIR/buildArmFinal.sh
ssh -i $ARM_KEY $ARM_USER@$ARM_HOST "bash $PUBLISHDIR/buildArmFinal.sh"

scp -i $X64_KEY buildIntelFinal.sh $X64_USER@$X64_HOST:$PUBLISHDIR/buildIntelFinal.sh
ssh -i $X64_KEY $X64_USER@$X64_HOST "bash $PUBLISHDIR/buildIntelFinal.sh"

# copy the builds back from the Linux machines
pushd ../../  # root of repository
scp -i $ARM_KEY $ARM_USER@$ARM_HOST:$PUBLISHDIR/tmbasic-linux-arm64.tar.gz dist/tmbasic-linux-arm64.tar.gz
scp -i $ARM_KEY $ARM_USER@$ARM_HOST:$PUBLISHDIR/tmbasic-linux-arm32.tar.gz dist/tmbasic-linux-arm32.tar.gz
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
echo
echo
ls -l ../../dist
echo "Finished at: $(date)"
