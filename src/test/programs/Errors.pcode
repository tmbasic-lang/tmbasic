Call 1
BranchIfNotError no_error
LoadErrorCodeA
NumberToString
StringPrint
LoadErrorMessageX
StringPrint
Exit
label no_error
LoadConstantStringX "no error"
StringPrint
Exit
-
# test that errors can be thrown from arbitrary stack positions
PushValues 5
PushObjects 10
LoadConstantStringX "this is the error message"
LoadConstantA 123
SetError
ClearX
LoadConstantA 0
ReturnIfError
