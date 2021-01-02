PushObjects 2

# x = { 10 => "AB", 20 => "CD", 30 => "EF" }
ValueToObjectMapNew
LoadConstantA 10
LoadConstantStringY "AB"
ValueToObjectMapSet
LoadConstantA 20
LoadConstantStringY "CD"
ValueToObjectMapSet
LoadConstantA 30
LoadConstantStringY "EF"
ValueToObjectMapSet

# x[30] = "ZZ"
LoadConstantA 30
LoadConstantStringY "ZZ"
ValueToObjectMapSet

# x.remove(30) : o0 = x
LoadConstantA 30
ValueToObjectMapRemove
StoreX 0

# x.count.print
ValueToObjectMapCount
NumberToString
StringPrint

# ",".print
LoadConstantStringX ","
StringPrint

# (x,b) = o0[10].tryget : x.print : ",".print : b.print : ",".print
LoadX 0
LoadConstantA 10
ValueToObjectMapTryGet
StringPrint
LoadConstantStringX ","
StringPrint
SetAFromB
NumberToString
StringPrint
LoadConstantStringX ","
StringPrint

# (x,b) = o0[20].tryget : x.print : ",".print : b.print : ",".print
LoadX 0
LoadConstantA 20
ValueToObjectMapTryGet
StringPrint
LoadConstantStringX ","
StringPrint
SetAFromB
NumberToString
StringPrint
LoadConstantStringX ","
StringPrint

# (x,b) = o0[30].tryget : b.print : ",".print
LoadX 0
LoadConstantA 30
ValueToObjectMapTryGet
SetAFromB
NumberToString
StringPrint
LoadConstantStringX ","
StringPrint

# o1 = o0.keys : o1.count.print : ",".print
LoadX 0
ValueToObjectMapKeys
StoreX 1
ValueListCount
NumberToString
StringPrint
LoadConstantStringX ","
StringPrint

# o1[0].print : ",".print
LoadX 1
LoadConstantA 0
ValueListGet
NumberToString
StringPrint
LoadConstantStringX ","
StringPrint

# o1[1].print : ",".print
LoadX 1
LoadConstantA 1
ValueListGet
NumberToString
StringPrint
LoadConstantStringX ","
StringPrint

# o1 = o0.values : o1.count.print : ",".print
LoadX 0
ValueToObjectMapValues
StoreX 1
ObjectListCount
NumberToString
StringPrint
LoadConstantStringX ","
StringPrint

# o1[0].print : ",".print
LoadX 1
LoadConstantA 0
ObjectListGet
StringPrint
LoadConstantStringX ","
StringPrint

# o1[1].print : ",".print
LoadX 1
LoadConstantA 1
ObjectListGet
StringPrint
LoadConstantStringX ","
StringPrint

Exit
