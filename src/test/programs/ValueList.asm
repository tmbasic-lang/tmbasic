# create this structure:
# [123, 456, 789]
ValueListBuilderBegin
LoadConstantA 999
ValueListBuilderAddA # remove this later to test ValueListRemove
LoadConstantA 123
ValueListBuilderAddA
LoadConstantA 999 # overwrite later to test ValueListSet
ValueListBuilderAddA
# insert 789 later to test ValueListInsert
ValueListBuilderEnd
LoadConstantA 0
ValueListRemove
LoadConstantA 1
LoadConstantB 456
ValueListSet
LoadConstantA 789
LoadConstantB 2
ValueListInsert
PushX
LoadConstantA 0
ValueListGet
NumberToString
StringPrint
LoadX 0
LoadConstantA 1
ValueListGet
NumberToString
StringPrint
LoadX 0
LoadConstantA 2
ValueListGet
NumberToString
StringPrint
LoadX 0
ValueListCount
NumberToString
StringPrint
Exit
