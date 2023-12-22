#procedure
function Foo() as Number
    while false
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
