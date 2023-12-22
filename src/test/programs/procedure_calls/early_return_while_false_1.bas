#procedure
function Foo() as Number
    while false
        return 1
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
