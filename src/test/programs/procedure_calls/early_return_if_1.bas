sub Main()
    print Foo(123)
end sub

function Foo(x as Number) as Number
    if x = 0 then
        return 1
    end if
    ' missing return here
end function

--output--
Compiler error
kControlReachesEndOfFunction
Foo
1:1
