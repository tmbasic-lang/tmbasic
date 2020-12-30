# create this structure:
# [123, 456]
ValueListBuilderBegin
LoadConstantA 123
ValueListBuilderAddA
LoadConstantA 999 # overwrite later to test ValueListSet
ValueListBuilderAddA
ValueListBuilderEnd
LoadConstantA 1
LoadConstantB 456
ValueListSet
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
Exit
