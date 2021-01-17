#!/bin/bash
# inputs: $OBJ_FILE $CXX
# takes the digits from the $OBJ_FILE filename and writes that many 'T' characters to a file. then builds that TTTT...
# file into an object using xxd.
set -euo pipefail
export NUM_BYTES=$(echo -n $OBJ_FILE | sed 's/[^0-9]//g')
export DATA_FILE=obj/pcode$NUM_BYTES
export SYMBOL=obj_pcode$NUM_BYTES
export SRC_FILE=obj/resources/pcode/$NUM_BYTES.cpp
head -c $NUM_BYTES /dev/zero | tr '\0' 'T' > $DATA_FILE
xxd -i $DATA_FILE | sed s/obj_pcode$NUM_BYTES/kResourcePcode/g > $SRC_FILE
$CXX -o $OBJ_FILE -c $SRC_FILE
