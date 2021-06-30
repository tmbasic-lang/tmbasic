#!/bin/bash
set -euo pipefail
TIMESTAMP=$(date --utc "+%Y%m%d%H%M%S")
FILENAME=terminfo-$TIMESTAMP.tar.gz

echo
echo "--------------------------------------------------------------------------------"
echo "Generating terminfo..."
cat Dockerfile.terminfo | docker buildx build -t terminfo . -f-
docker create --name terminfo terminfo
docker export terminfo > terminfo-temp.tar
docker container rm terminfo
docker image rm terminfo
TERMINFO_DIR=$PWD
rm -rf temp
mkdir -p temp
pushd temp
tar xf ../terminfo-temp.tar
cd /usr/share/terminfo
tar zcf $TERMINFO_DIR/$FILENAME *
popd
aws s3 cp $FILENAME s3://tmbasic/linux-terminfo/$FILENAME --acl public-read
rm -rf terminfo-temp.tar temp $FILENAME
echo "--------------------------------------------------------------------------------"
echo
