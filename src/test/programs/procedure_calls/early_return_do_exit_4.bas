sub Main()
    print Foo(123)
end sub

function Foo(x as Number) as Number
    do
        exit do
        throw 999, "blah"
    loop while x = 5
end function

--output--
Compiler error
kControlReachesEndOfFunction
Foo
1:1
