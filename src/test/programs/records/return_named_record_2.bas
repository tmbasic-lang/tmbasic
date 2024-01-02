type Foo
    a as Number
end type

sub Main()
    dim foo as Foo
    foo = Bar()
    print foo.a
end sub

function Bar() as Foo
    return { a: 1 } as Foo
end function

--output--
1
