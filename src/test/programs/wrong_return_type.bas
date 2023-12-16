#procedure
function Foo() as Number
    return "hello"
end function

#procedure
sub Main()
    print Foo()
end sub

--output--
Compiler error
kTypeMismatch
Foo
2:5
