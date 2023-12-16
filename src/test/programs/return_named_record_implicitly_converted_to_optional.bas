#type
type MyType
end type

#procedure
function Foo() as Optional MyType
    dim x as MyType
    return x
end function

#procedure
sub Main()
    dim x = Value(Foo())
end sub

--output--
