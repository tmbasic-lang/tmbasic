# "ABC".indexof("BC",0).print : ",".print
LoadConstantStringX "ABC"
LoadConstantStringY "BC"
LoadConstantA 0
StringIndexOf
NumberToString
StringPrint
LoadConstantStringX ","
StringPrint

# "ABC".indexof("BC",2).print : ",".print
LoadConstantStringX "ABC"
LoadConstantStringY "BC"
LoadConstantA 2
StringIndexOf
NumberToString
StringPrint
LoadConstantStringX ","
StringPrint

# "ABC".indexof("BC",-1).print : ",".print
LoadConstantStringX "ABC"
LoadConstantStringY "BC"
LoadConstantA -1
StringIndexOf
NumberToString
StringPrint
LoadConstantStringX ","
StringPrint

# "ABC".indexof("",0).print : ",".print
LoadConstantStringX "ABC"
LoadConstantStringY ""
LoadConstantA 0
StringIndexOf
NumberToString
StringPrint
LoadConstantStringX ","
StringPrint

Exit
