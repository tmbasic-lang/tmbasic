#procedure
sub Main()
    print "hello"
    print Foo(5)
end sub
#procedure
function Foo(a as Number) as Number
    return a
end function
--output--
hello
5
