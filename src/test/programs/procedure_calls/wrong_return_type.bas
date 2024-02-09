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
The return type of this function is Number, but the "return" statement expression is of type String.
