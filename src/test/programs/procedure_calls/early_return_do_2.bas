#procedure
sub Main()
    print Foo(123)
end sub

#procedure
function Foo(x as Number) as Number
    do
        return 1
    loop while x = 5
end function

--output--
1
