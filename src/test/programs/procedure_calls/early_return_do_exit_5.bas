sub Main()
    print Foo(123)
end sub

function Foo(x as Number) as Number
    do
        if x = 1 then
            exit do
        end if
        throw 999, "blah"
    loop while x = 5
end function

--output--
Compiler error
kControlReachesEndOfFunction
Foo
1:1
Control may reach the end of this function without returning or throwing.
