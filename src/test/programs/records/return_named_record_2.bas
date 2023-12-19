#type
type Foo
    a as Number
end type

#procedure
sub Main()
    dim foo as Foo
    foo = Bar()
    print foo.a
end sub

#procedure
function Bar() as Foo
    return { a: 1 } as Foo
end function

--output--
1
