#procedure
function Foo() as Number
    ' At runtime this would return. However, we aren't smart enough to know at runtime that
    ' while true will execute the loop body at least once.
    while true
        throw 999, "blah"
    wend
end function

#procedure
sub Main()
end sub

--output--
Compiler error
kControlReachesEndOfFunction
Foo
1:1
