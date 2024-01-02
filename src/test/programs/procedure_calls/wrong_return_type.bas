function Foo() as Number
    return "hello"
end function

sub Main()
    print Foo()
end sub

--output--
Compiler error
kTypeMismatch
Foo
2:5
