#procedure
sub Main()
    print Foo(123)
end sub

#procedure
function Foo(x as Number) as Number
    do
    loop while x = 5
end function

--output--
Compiler error
kControlReachesEndOfFunction
Foo
1:1
