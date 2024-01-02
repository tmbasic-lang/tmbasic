type MyRecord
    a as Number
    b as String
end type
function Foo() as MyRecord
    return { a: 5, b: "hello" } as MyRecord
end function
sub Main()
    dim x = Foo()
    print x.a
    print x.b
end sub
--output--
5
hello
