sub Main()
    print Foo(123)
end sub

function Foo(x as Number) as Number
    try
        return 1
    catch
    end try
end function

--output--
Compiler error
kControlReachesEndOfFunction
Foo
1:1
Control may reach the end of this function without returning or throwing.
