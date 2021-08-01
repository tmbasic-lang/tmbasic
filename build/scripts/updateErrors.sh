#!/bin/bash
# updates BuiltInConstantList.cpp and ref_constants.txt based on the errors in src/vm/Error.h

# src/compiler/BuiltInConstantList.cpp
grep -v ERR_ src/compiler/BuiltInConstantList.cpp > /tmp/list.cpp
export LINE_NUMBER=$(grep -n "// start_errors" /tmp/list.cpp | awk -F: '{ print $1 }')
head -n $LINE_NUMBER /tmp/list.cpp > /tmp/list2.cpp
grep ERR_ src/vm/Error.h | awk '{ print "    addError(\"" $5 "\", vm::ErrorCode::" $1 ");" }' >> /tmp/list2.cpp
tail -n +$(($LINE_NUMBER+1)) /tmp/list.cpp >> /tmp/list2.cpp
mv -f /tmp/list2.cpp src/compiler/BuiltInConstantList.cpp
rm -f /tmp/list.cpp

# doc/help/topics/ref_constants.txt
grep -v "li@" doc/help/topics/ref_constants.txt > /tmp/help.txt
export LINE_NUMBER=$(grep -n "ul@" /tmp/help.txt | awk -F: '{ print $1 }')
head -n $LINE_NUMBER /tmp/help.txt > /tmp/help2.txt
grep ERR_ src/vm/Error.h | awk '{ print "li@`" $5 "`@" }' | sort >> /tmp/help2.txt
tail -n +$(($LINE_NUMBER+1)) /tmp/help.txt >> /tmp/help2.txt
mv -f /tmp/help2.txt doc/help/topics/ref_constants.txt
rm -f /tmp/help.txt
