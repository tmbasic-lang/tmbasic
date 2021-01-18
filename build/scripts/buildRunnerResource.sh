#!/bin/bash
# inputs: $OBJ_FILE $CXX
set -euo pipefail
export NAME=$(basename $OBJ_FILE .o)
export BIN_FILE=obj/resources/runners/$NAME
export CPP_FILE=obj/resources/runners/$NAME.cpp
export OLD_SYMBOL=obj_resources_runners_$NAME
export NEW_SYMBOL=$(echo -n kResource_runner_$NAME | sed 's/\./_/g')
xxd -i $BIN_FILE | sed s/$OLD_SYMBOL/$NEW_SYMBOL/g > $CPP_FILE
$CXX -o $OBJ_FILE -c $CPP_FILE
