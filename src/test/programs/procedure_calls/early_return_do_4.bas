#procedure
sub Main()
    print Foo(123)
end sub

#procedure
function Foo(x as Number) as Number
    do
        throw 999, "blah"
    loop while x = 5
end function

--output--
Error
999
blah
