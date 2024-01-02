type Foo
    a as Number
end type

sub Main()
    print Bar().a
end sub

function Bar() as Foo
    return { a: 1 } as Foo
end function

--output--
1
