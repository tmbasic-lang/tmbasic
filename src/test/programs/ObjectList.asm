# create this structure:
# ["ABC", {v0:123}, [456,789], ["DEF","GHI"]]
ObjectListBuilderBegin
LoadConstantStringX "ZZZ" # overwrite later to test ObjectListSet
ObjectListBuilderAddX

RecordBuilderBegin 1 0
LoadConstantA 123
RecordBuilderStoreA
RecordBuilderEnd
ObjectListBuilderAddX

ValueListBuilderBegin
LoadConstantA 456
ValueListBuilderAddA
LoadConstantA 789
ValueListBuilderAddA
ValueListBuilderEnd
ObjectListBuilderAddX

ObjectListBuilderBegin
LoadConstantStringX "DEF"
ObjectListBuilderAddX
LoadConstantStringX "GHI"
ObjectListBuilderAddX
ObjectListBuilderEnd
ObjectListBuilderAddX

ObjectListBuilderEnd
LoadConstantA 0
LoadConstantStringX "ABC"
ObjectListSet
PushX

LoadConstantA 0
ObjectListGet
StringPrint

LoadX 0
LoadConstantA 1
ObjectListGet
RecordLoadA 0
NumberToString
StringPrint

LoadX 0
LoadConstant 2
ObjectListGet
LoadConstantA 0
ValueListGet
NumberToString
StringPrint

LoadX 0
LoadConstant 2
ObjectListGet
LoadConstantA 1
ValueListGet
NumberToString
StringPrint

LoadX 0
LoadConstant 3
ObjectListGet
LoadConstantA 0
ObjectListGet
StringPrint

LoadX 0
LoadConstant 3
ObjectListGet
LoadConstantA 1
ObjectListGet
StringPrint

Exit
