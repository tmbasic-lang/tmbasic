#procedure
sub Main()
    print Foo(123)
end sub

#procedure
function Foo(x as Number) as Number
    do
        if x = 1 then
            exit do
        end if
        return 1
    loop while x = 5
end function

--output--
Compiler error
kControlReachesEndOfFunction
Foo
1:1
