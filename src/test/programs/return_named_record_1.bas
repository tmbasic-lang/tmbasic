#type
type Foo
    a as Number
end type

#procedure
sub Main()
    print Bar().a
end sub

#procedure
function Bar() as Foo
    return { a: 1 } as Foo
end function

--output--
1
