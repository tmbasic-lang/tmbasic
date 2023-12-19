#procedure
sub Main()
    print Foo(5, 6)
end sub
#procedure
function Foo(a as Number, b as Number) as Number
    return a + b
end function
--output--
11
