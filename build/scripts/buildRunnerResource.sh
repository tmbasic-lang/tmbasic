#!/bin/bash
# inputs: $OBJ_FILE $CXX
set -euo pipefail
export PLATFORM=$(basename $OBJ_FILE .o)
export BIN_FILE=obj/resources/runners/$PLATFORM
export CPP_FILE=obj/resources/runners/$PLATFORM.cpp
export OLD_SYMBOL=obj_runners_$PLATFORM
export NEW_SYMBOL=kResource_runner_$PLATFORM
xxd -i $BIN_FILE | sed s/$OLD_SYMBOL/$NEW_SYMBOL/g > $CPP_FILE
$CXX -o $OBJ_FILE -c $CPP_FILE
