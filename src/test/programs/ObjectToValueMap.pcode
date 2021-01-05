PushObjects 1

# o0 = { "A"=>1 }
ObjectToValueMapNew
LoadConstantStringY "A"
LoadConstantA 1
ObjectToValueMapSet
StoreX 0

# o0 = o0.add({v0:2}, 3)
RecordBuilderBegin 1 0
LoadConstantA 2
RecordBuilderStoreA 0
RecordBuilderEnd
SetYFromX
LoadX 0
LoadConstantA 3
ObjectToValueMapSet
StoreX 0

# o0 = o0.add([4,5], 6)
ValueListBuilderBegin
LoadConstantA 4
ValueListBuilderAddA
LoadConstantA 5
ValueListBuilderAddA
ValueListBuilderEnd
SetYFromX
LoadX 0
LoadConstantA 6
ObjectToValueMapSet
StoreX 0

# o0 = o0.add("Z", 9)
LoadX 0
LoadConstantStringY "Z"
LoadConstantA 9
ObjectToValueMapSet
StoreX 0

# o0 = o0.remove("Z")
LoadX 0
LoadConstantStringY "Z"
ObjectToValueMapRemove
StoreX 0

# o0.count.print : ",".print
LoadX 0
ObjectToValueMapCount
NumberToString
StringPrint
LoadConstantStringX ","
StringPrint

# (a,b) = o0["A"] : a.print : ",".print : b.print : ",".print
LoadX 0
LoadConstantStringY "A"
ObjectToValueMapTryGet
NumberToString
StringPrint
LoadConstantStringX ","
StringPrint
SetAFromB
NumberToString
StringPrint
LoadConstantStringX ","
StringPrint

# (a,b) = o0[{v0:2}] : a.print : ",".print : b.print : ",".print
RecordBuilderBegin 1 0
LoadConstantA 2
RecordBuilderStoreA 0
RecordBuilderEnd
SetYFromX
LoadX 0
ObjectToValueMapTryGet
NumberToString
StringPrint
LoadConstantStringX ","
StringPrint
SetAFromB
NumberToString
StringPrint
LoadConstantStringX ","
StringPrint

# (a,b) = o0[[4,5]] : a.print : ",".print : b.print : ",".print
ValueListBuilderBegin
LoadConstantA 4
ValueListBuilderAddA
LoadConstantA 5
ValueListBuilderAddA
ValueListBuilderEnd
SetYFromX
LoadX 0
ObjectToValueMapTryGet
NumberToString
StringPrint
LoadConstantStringX ","
StringPrint
SetAFromB
NumberToString
StringPrint
LoadConstantStringX ","
StringPrint

# (a,b) = o0["Z"] : b.print : ",".print
LoadConstantStringY "Z"
LoadX 0
ObjectToValueMapTryGet
SetAFromB
NumberToString
StringPrint
LoadConstantStringX ","
StringPrint

# x = o0.keys : x.count.print : ",".print
LoadX 0
ObjectToValueMapKeys
ObjectListCount
NumberToString
StringPrint
LoadConstantStringX ","
StringPrint

# x = o0.values : x.count.print : ",".print
LoadX 0
ObjectToValueMapValues
ValueListCount
NumberToString
StringPrint
LoadConstantStringX ","
StringPrint

Exit
