#!/bin/bash
# updates src/compiler/BuiltInConstantList.cpp based on the errors in src/vm/Error.h

grep -v ERR_ src/compiler/BuiltInConstantList.cpp > /tmp/list.cpp
export LINE_NUMBER=$(grep -n "// start_errors" /tmp/list.cpp | awk -F: '{ print $1 }')
head -n $LINE_NUMBER /tmp/list.cpp > /tmp/list2.cpp
cat src/vm/Error.h | grep ERR_ | awk '{ print "    addError(\"" $5 "\", vm::ErrorCode::" $1 ");" }' >> /tmp/list2.cpp
tail -n +$(($LINE_NUMBER+1)) /tmp/list.cpp >> /tmp/list2.cpp
mv -f /tmp/list2.cpp src/compiler/BuiltInConstantList.cpp
rm -f /tmp/list.cpp
