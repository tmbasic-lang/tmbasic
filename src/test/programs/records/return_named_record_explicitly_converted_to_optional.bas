type MyType
end type

function Foo() as Optional MyType
    dim x as MyType
    return x as Optional MyType
end function

sub Main()
    dim x = Value(Foo())
end sub

--output--
