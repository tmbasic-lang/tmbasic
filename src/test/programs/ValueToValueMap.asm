PushObjects 2

# x = { 1 => 111, 2 => 222 }
ValueToValueMapNew
LoadConstantA 1
LoadConstantB 111
ValueToValueMapSet
LoadConstantA 2
LoadConstantB 222
ValueToValueMapSet

# x[3] = 333
LoadConstantA 3
LoadConstantB 333
ValueToValueMapSet

# x[3] = 444
LoadConstantB 444
ValueToValueMapSet

# x.remove(3)
ValueToValueMapRemove

# o0 = x
StoreX 0

# print o0.count
LoadX 0
ValueToValueMapCount
NumberToString
StringPrint

# (a,b) = o0.tryget(1) : print a : print b
LoadX 0
LoadConstantA 1
ValueToValueMapTryGet
NumberToString
StringPrint
SetAFromB
NumberToString
StringPrint

# (a,b) = o0.tryget(2) : print a : print b
LoadX 0
LoadConstantA 2
ValueToValueMapTryGet
NumberToString
StringPrint
SetAFromB
NumberToString
StringPrint

# (a,b) = o0.tryget(999) : print b
LoadX 0
LoadConstantA 999
ValueToValueMapTryGet
SetAFromB
NumberToString
StringPrint

# o1 = o0.keys : o1.count.print
LoadX 0
ValueToValueMapKeys
StoreX 1
ValueListCount
NumberToString
StringPrint

# o1[0].print
LoadX 1
LoadConstantA 0
ValueListGet
NumberToString
StringPrint

# o1[1].print
LoadX 1
LoadConstantA 1
ValueListGet
NumberToString
StringPrint

# o1 = o0.values : o1.count.print
LoadX 0
ValueToValueMapValues
StoreX 1
ValueListCount
NumberToString
StringPrint

# o1[0].print
LoadX 1
LoadConstantA 0
ValueListGet
NumberToString
StringPrint

# o1[1].print
LoadX 1
LoadConstantA 1
ValueListGet
NumberToString
StringPrint

Exit
