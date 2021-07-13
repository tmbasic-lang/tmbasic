#procedure
sub Main()
    print "hello"
    print Foo(Bar(999))
end sub
#procedure
function Foo(a as Number) as Number
    return a
end function
#procedure
function Bar(ignored as Number) as Number
    return 5
end function
--output--
hello
5
